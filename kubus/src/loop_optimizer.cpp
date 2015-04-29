#include <qbb/kubus/loop_optimizer.hpp>

#include <qbb/kubus/lower_abstract_indices.hpp>

#include <qbb/kubus/IR/pretty_printer.hpp>

#include <qbb/kubus/IR/extract.hpp>

#include <qbb/kubus/IR/kir.hpp>
#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/IR.hpp>

#include <qbb/kubus/isl/context.hpp>
#include <qbb/kubus/isl/set.hpp>
#include <qbb/kubus/isl/map.hpp>
#include <qbb/kubus/isl/constraint.hpp>
#include <qbb/kubus/isl/multi_union_pw_affine_expr.hpp>
#include <qbb/kubus/isl/flow.hpp>
#include <qbb/kubus/isl/schedule.hpp>
#include <qbb/kubus/isl/ast_builder.hpp>

#include <qbb/util/unique_name_generator.hpp>
#include <qbb/util/integers.hpp>
#include <qbb/util/numeric/bisection.hpp>
#include <qbb/util/numeric/polynomial.hpp>
#include <qbb/util/unused.hpp>

#include <boost/variant.hpp>

#include <vector>
#include <map>
#include <string>
#include <tuple>
#include <cassert>

#include <qbb/kubus/IR/pretty_printer.hpp>
#include <iostream>
#include <qbb/kubus/isl/printer.hpp>

namespace qbb
{
namespace kubus
{

namespace
{

isl::context isl_ctx;

isl::union_map pad_schedule_map(isl::union_map schedule_map)
{
    auto result = isl::union_map::empty(schedule_map.get_space());

    auto maps = schedule_map.get_maps();

    int max_out_dim = 0;

    for (const auto& map : maps)
    {
        max_out_dim = std::max(max_out_dim, map.dim(isl_dim_out));
    }

    for (auto map : maps)
    {
        int n_out = map.dim(isl_dim_out);

        map = add_dims(map, isl_dim_out, max_out_dim - n_out);
        for (int i = n_out; i < max_out_dim; ++i)
            map = fix_dimension(map, isl_dim_out, i, 0);

        result = add_map(result, map);
    }

    return result;
}

struct scop
{
    scop(isl::union_set domain, isl::set QBB_UNUSED(param_constraints), isl::union_map schedule,
         isl::union_map write_accesses, isl::union_map read_accesses,
         std::map<std::string, expression> symbol_table)
    : schedule(isl::schedule::from_domain(domain)), write_accesses(write_accesses),
      read_accesses(read_accesses), symbol_table(symbol_table)
    {
        auto part_sched =
            isl::multi_union_pw_affine_expr::from_union_map(pad_schedule_map(schedule));

        isl::schedule_node root = this->schedule.get_root();

        auto new_ = insert_partial_schedule(root[0], part_sched);

        this->schedule = get_schedule(new_);
    }

    isl::schedule schedule;

    isl::union_map write_accesses;
    isl::union_map read_accesses;

    std::map<std::string, expression> symbol_table;
};

struct scop_ctx
{
    scop_ctx() : domain(isl::set::universe(isl::space(isl_ctx, 0, 0))), scatter_index{0}
    {
    }

    isl::set domain;
    std::vector<long int> scatter_index;
    std::vector<variable_declaration> indices;
};

struct scop_info
{
public:
    scop_info()
    : param_constraints(isl::set::universe(isl::space(isl_ctx, 0))),
      domain(isl::union_set::empty(isl::space(isl_ctx, 0, 0))),
      schedule(isl::union_map::empty(isl::space(isl_ctx, 0, 0, 0))),
      write_accesses(isl::union_map::empty(isl::space(isl_ctx, 0, 0))),
      read_accesses(isl::union_map::empty(isl::space(isl_ctx, 0, 0)))
    {
    }

    void add_write_accesses(isl::union_map write_accesses)
    {
        this->write_accesses = union_(this->write_accesses, write_accesses);
    }

    void add_read_accesses(isl::union_map read_accesses)
    {
        this->read_accesses = union_(this->read_accesses, read_accesses);
    }

    const std::string& map_tensor_to_name(util::handle id) const
    {
        auto iter = tensor_table.find(id);

        if (iter != tensor_table.end())
        {
            return iter->second;
        }
        else
        {
            return tensor_table.emplace(id, get_new_symbol()).first->second;
        }
    }

    const std::string& map_index_to_name(util::handle id) const
    {
        auto iter = index_table.find(id);

        if (iter != index_table.end())
        {
            return iter->second;
        }
        else
        {
            return index_table.emplace(id, get_new_symbol()).first->second;
        }
    }

    std::string add_symbol(expression expr)
    {
        std::string id = get_new_symbol();

        symbol_table.emplace(id, expr);

        return id;
    }

    void add_partial_schedule(isl::set partial_domain, isl::map partial_schedule)
    {
        domain = isl::add_set(domain, partial_domain);
        schedule = isl::add_map(schedule, partial_schedule);
    }

    void add_param_constraint(isl::basic_set constraint)
    {
        param_constraints = intersect_params(param_constraints, constraint);
    }

    scop build_scop() const
    {
        return scop(domain, param_constraints, schedule, write_accesses, read_accesses,
                    symbol_table);
    }

private:
    std::string get_new_symbol() const
    {
        return name_gen.get();
    }

    isl::set param_constraints;

    isl::union_set domain;
    isl::union_map schedule;

    isl::union_map write_accesses;
    isl::union_map read_accesses;

    std::map<std::string, expression> symbol_table;

    mutable std::map<util::handle, std::string> tensor_table;
    mutable std::map<util::handle, std::string> index_table;

    util::unique_name_generator name_gen;
};

using bound = boost::variant<util::index_t, std::string>;

bound extract_bound(const expression& expr, scop_info& ctx)
{
    pattern::variable<util::index_t> value;
    pattern::variable<expression> a;

    auto m = pattern::make_matcher<expression, bound>()
                 .case_(integer_literal(value),
                        [&]
                        {
                            return bound(value.get());
                        })
                 .case_(a, [&]
                        {
                            auto name = ctx.add_symbol(a.get());

                            isl::space s(isl_ctx, 1);
                            s.set_dim_name(isl_dim_param, 0, name);

                            auto param_constraint = isl::basic_set::universe(s);

                            auto c =
                                isl::constraint::inequality(s).set_coefficient(isl_dim_param, 0, 1);

                            param_constraint.add_constraint(c);

                            ctx.add_param_constraint(param_constraint);

                            return bound(name);
                        });

    return pattern::match(expr, m);
}

struct local_domain_builder : boost::static_visitor<isl::basic_set>
{
public:
    explicit local_domain_builder(std::string idx_name_) : idx_name_(idx_name_)
    {
    }

    isl::basic_set operator()(int lower_bound, int upper_bound) const
    {
        isl::space space(isl_ctx, 0, 1);

        space.set_dim_name(isl_dim_set, 0, idx_name_);

        auto local_domain = isl::basic_set::universe(space);

        auto lower_bound_constraint = isl::constraint::inequality(space)
                                          .set_coefficient(isl_dim_set, 0, 1)
                                          .set_constant(lower_bound);

        auto upper_bound_constraint = isl::constraint::inequality(space)
                                          .set_coefficient(isl_dim_set, 0, -1)
                                          .set_constant(upper_bound - 1);

        local_domain.add_constraint(lower_bound_constraint);
        local_domain.add_constraint(upper_bound_constraint);

        return local_domain;
    }

    isl::basic_set operator()(int lower_bound, const std::string& upper_bound) const
    {
        isl::space space(isl_ctx, 1, 1);

        space.set_dim_name(isl_dim_param, 0, upper_bound);

        space.set_dim_name(isl_dim_set, 0, idx_name_);

        auto local_domain = isl::basic_set::universe(space);

        auto lower_bound_constraint = isl::constraint::inequality(space)
                                          .set_coefficient(isl_dim_set, 0, 1)
                                          .set_constant(lower_bound);

        auto upper_bound_constraint = isl::constraint::inequality(space)
                                          .set_coefficient(isl_dim_set, 0, -1)
                                          .set_coefficient(isl_dim_param, 0, 1)
                                          .set_constant(-1);

        local_domain.add_constraint(lower_bound_constraint);
        local_domain.add_constraint(upper_bound_constraint);

        return local_domain;
    }

    isl::basic_set operator()(const std::string& lower_bound, int upper_bound) const
    {
        isl::space space(isl_ctx, 1, 1);

        space.set_dim_name(isl_dim_param, 0, lower_bound);

        space.set_dim_name(isl_dim_set, 0, idx_name_);

        auto local_domain = isl::basic_set::universe(space);

        auto lower_bound_constraint = isl::constraint::inequality(space)
                                          .set_coefficient(isl_dim_set, 0, 1)
                                          .set_coefficient(isl_dim_param, 0, 1);

        auto upper_bound_constraint = isl::constraint::inequality(space)
                                          .set_coefficient(isl_dim_set, 0, -1)
                                          .set_constant(upper_bound - 1);

        local_domain.add_constraint(lower_bound_constraint);
        local_domain.add_constraint(upper_bound_constraint);

        return local_domain;
    }

    isl::basic_set operator()(const std::string& lower_bound, const std::string& upper_bound) const
    {
        isl::space space(isl_ctx, 2, 1);

        space.set_dim_name(isl_dim_param, 0, lower_bound);
        space.set_dim_name(isl_dim_param, 1, upper_bound);

        space.set_dim_name(isl_dim_set, 0, idx_name_);

        auto local_domain = isl::basic_set::universe(space);

        auto lower_bound_constraint = isl::constraint::inequality(space)
                                          .set_coefficient(isl_dim_set, 0, 1)
                                          .set_coefficient(isl_dim_param, 0, 1);

        auto upper_bound_constraint = isl::constraint::inequality(space)
                                          .set_coefficient(isl_dim_set, 0, -1)
                                          .set_coefficient(isl_dim_param, 1, 1)
                                          .set_constant(-1);

        local_domain.add_constraint(lower_bound_constraint);
        local_domain.add_constraint(upper_bound_constraint);

        return local_domain;
    }

private:
    std::string idx_name_;
};

bool is_scop(const expression&)
{
    return true;
}

isl::union_map analyze_accesses(const expression& expr, const isl::set& domain, scop_info& ctx)
{
    pattern::variable<variable_declaration> decl;
    pattern::variable<std::vector<expression>> indices;

    isl::space no_access_space(isl_ctx, 0, 0);
    isl::union_map accesses = isl::union_map::empty(no_access_space);

    auto m = pattern::make_matcher<expression, void>().case_(
        subscription(tensor(decl), indices), [&]
        {
            auto tensor_name = ctx.map_tensor_to_name(decl.get().id());

            auto number_of_indices = indices.get().size();

            isl::space access_space(isl_ctx, 0, number_of_indices);

            access_space.set_tuple_name(isl_dim_set, tensor_name);

            auto range = isl::set::universe(access_space);

            auto access = isl::make_map_from_domain_and_range(domain, range);

            for (std::size_t i = 0; i < number_of_indices; ++i)
            {
                auto decl = indices.get()[i].as<variable_ref_expr>().declaration();

                const auto& idx_name = ctx.map_index_to_name(decl.id());

                auto idx = domain.get_space().find_dim_by_name(isl_dim_set, idx_name);

                access.add_constraint(isl::constraint::equality(access.get_space())
                                          .set_coefficient(isl_dim_out, i, 1)
                                          .set_coefficient(isl_dim_in, idx, -1));
            }

            accesses = add_map(accesses, access);
        });

    pattern::for_each(expr, m);

    return accesses;
}

void analyze_tensor_accesses(const expression& expr, const isl::set& domain, scop_info& ctx)
{
    pattern::variable<expression> lhs, rhs;

    auto m = pattern::make_matcher<expression, void>()
                 .case_(binary_operator(pattern::value(binary_op_tag::assign), lhs, rhs),
                        [&]
                        {
                            auto write_accesses = analyze_accesses(lhs.get(), domain, ctx);
                            auto read_accesses = analyze_accesses(rhs.get(), domain, ctx);

                            ctx.add_write_accesses(write_accesses);
                            ctx.add_read_accesses(read_accesses);
                        })
                 .case_(binary_operator(pattern::value(binary_op_tag::plus_assign), lhs, rhs), [&]
                        {
                            auto readwrite_accesses = analyze_accesses(lhs.get(), domain, ctx);
                            auto read_accesses = analyze_accesses(rhs.get(), domain, ctx);

                            ctx.add_write_accesses(readwrite_accesses);
                            ctx.add_read_accesses(read_accesses);
                            ctx.add_read_accesses(readwrite_accesses);
                        });

    pattern::for_each(expr, m);
}

void analyze_scop_(const expression& expr, scop_info& info, scop_ctx ctx)
{
    using pattern::_;

    pattern::variable<expression> a, b, c;
    pattern::variable<variable_declaration> idx;

    pattern::variable<std::vector<expression>> subexprs;

    auto m =
        pattern::make_matcher<expression, void>()
            .case_(for_(idx, a, b, c),
                   [&]
                   {
                       auto lower_bound = extract_bound(a.get(), info);
                       auto upper_bound = extract_bound(b.get(), info);

                       std::string idx_name = info.map_index_to_name(idx.get().id());
                       ctx.indices.push_back(idx.get());

                       auto local_domain = boost::apply_visitor(local_domain_builder(idx_name),
                                                                lower_bound, upper_bound);

                       // add dimension to the global iteration space
                       ctx.domain = flat_product(ctx.domain, local_domain);

                       // add a zero to the scatter index
                       ctx.scatter_index.push_back(0);

                       // analyze loop body
                       analyze_scop_(c.get(), info, ctx);
                   })
            .case_(compound(subexprs),
                   [&]
                   {
                       for (const auto& sub_expr : subexprs.get())
                       {
                           analyze_scop_(sub_expr, info, ctx);
                           ++ctx.scatter_index.back();
                       }
                   })
            .case_(
                a, [&]
                {
                    // we entered the loop body

                    isl::set domain = ctx.domain;

                    auto name = info.add_symbol(macro_expr(ctx.indices, a.get()));

                    domain.set_tuple_name(name);

                    long int number_of_indices = domain.get_space().dim(isl_dim_set);
                    long int number_of_dimensions = 2 * number_of_indices + 1;

                    assert(number_of_indices + 1 == util::to_uindex(ctx.scatter_index.size()));

                    isl::set scattering_domain =
                        isl::set::universe(drop_all_dims(domain.get_space(), isl_dim_param));

                    isl::space scattering_range_space(isl_ctx, 0, number_of_dimensions);
                    scattering_range_space.set_tuple_name(isl_dim_set, "scattering");

                    isl::set scattering_range = isl::set::universe(scattering_range_space);

                    isl::map scattering =
                        make_map_from_domain_and_range(scattering_domain, scattering_range);

                    scattering.add_constraint(isl::constraint::equality(scattering.get_space())
                                                  .set_coefficient(isl_dim_out, 0, 1)
                                                  .set_constant(-ctx.scatter_index[0]));
                    for (long int i = 0; i < number_of_indices; ++i)
                    {
                        scattering.add_constraint(isl::constraint::equality(scattering.get_space())
                                                      .set_coefficient(isl_dim_out, 2 * i + 2, 1)
                                                      .set_constant(-ctx.scatter_index[i + 1]));
                        scattering.add_constraint(isl::constraint::equality(scattering.get_space())
                                                      .set_coefficient(isl_dim_out, 2 * i + 1, 1)
                                                      .set_coefficient(isl_dim_in, i, -1));
                    }

                    analyze_tensor_accesses(a.get(), domain, info);

                    info.add_partial_schedule(domain, scattering);

                    // TODO: add a symbol for the loop body
                });

    pattern::match(expr, m);
}

scop analyze_scop(const expression& expr)
{
    scop_info info;

    scop_ctx ctx;

    analyze_scop_(expr, info, ctx);

    scop s = info.build_scop();

    return s;
}

isl::map turn_dims_into_params(isl::map m, isl_dim_type type, int pos, int n)
{
    //TODO: Index bounds

    //TODO: Assert pos >= 0.

    auto n_dim = m.dim(type);

    //TODO: Assert pos + n < n_dim.

    isl::space s(m.get_ctx(), n, n, n_dim - n);

    for (int i = pos; i < pos + n; ++i)
    {
        s.set_dim_name(isl_dim_param, i, "o" + std::to_string(i));
    }

    auto dims_to_params = isl::basic_map::universe(s);

    for (int i = 0; i < pos; ++i)
    {
        auto c = isl::constraint::equality(s)
                .set_coefficient(isl_dim_in, i, 1)
                .set_coefficient(isl_dim_out, i, -1);

        dims_to_params.add_constraint(c);
    }

    for (int i = pos; i < pos + n; ++i)
    {
        auto c = isl::constraint::equality(s)
                .set_coefficient(isl_dim_in, i, 1)
                .set_coefficient(isl_dim_param, i, -1);

        dims_to_params.add_constraint(c);
    }

    for (int i = pos + n; i < n_dim; ++i)
    {
        auto c = isl::constraint::equality(s)
                .set_coefficient(isl_dim_in, i, 1)
                .set_coefficient(isl_dim_out, i, -1);

        dims_to_params.add_constraint(c);
    }

    dims_to_params = align_params(dims_to_params, m.get_space());

    switch (type)
    {
        case isl_dim_in:
            return apply_domain(m, dims_to_params);
        case isl_dim_out:
            return apply_range(m, dims_to_params);
        default:
            throw 0; //TODO: Throw exception.
    }
}

isl::schedule_node get_optimized_schedule_tree(isl::schedule_node root, scop& s)
{
    int n_children = root.n_children();

    for (int i = 0; i < n_children; ++i)
    {
        auto child = root[i];

        root = get_optimized_schedule_tree(child, s).parent();
    }

    if (root.get_type() == isl_schedule_node_band)
    {
        if (root.band_is_permutable())
        {
            if (root[0].get_type() == isl_schedule_node_leaf)
            {
                isl_options_set_tile_shift_point_loops(isl_ctx.native_handle(), 0);
                // isl_options_set_tile_scale_tile_loops(isl_ctx.native_handle(), 0);

                if (root.band_n_member() > 1)
                {
                    std::vector<util::index_t> tile_sizes = {300/*, 100, 25*/};

                    isl::schedule_node band_to_tile = root;

                    for (auto tile_size : tile_sizes)
                    {
                        std::vector<util::index_t> tile_shape(root.band_n_member(), tile_size);

                        auto the_tile_band = tile_band(band_to_tile, tile_shape);

                        band_to_tile = the_tile_band[0];

                        band_to_tile = insert_mark(band_to_tile, isl::id(isl_ctx, "task"))[0];

                        auto prefix_schedule = band_to_tile.get_prefix_schedule_union_map();

                        //auto extension = reverse(apply_domain(prefix_schedule, s.write_accesses));
                        //isl_union_map_dump(extension.native_handle());

                        //band_to_tile = graft_before(band_to_tile, isl::schedule_node::from_extension(extension));
                    }

                    root = band_to_tile;

                    for (std::size_t i = 0; i < tile_sizes.size(); ++i)
                    {
                        root = root.parent().parent();
                    }
                }
            }
        }
    }
    
    return root;
}

scop optimize_scop(scop s)
{
    isl::union_map schedule = s.schedule.get_map();

    isl::union_map read = s.read_accesses;
    isl::union_map write = s.write_accesses;

    isl::union_map may_write = isl::union_map::empty(isl::space(isl_ctx, 0, 0, 0));

    isl::union_map dummy = isl::union_map::empty(isl::space(isl_ctx, 0, 0, 0));

    isl::union_map raw = isl::union_map::empty(isl::space(isl_ctx, 0, 0, 0));
    isl::union_map waw = isl::union_map::empty(isl::space(isl_ctx, 0, 0, 0));
    isl::union_map war = isl::union_map::empty(isl::space(isl_ctx, 0, 0, 0));

    isl::compute_flow(read, write, may_write, schedule, raw, dummy, dummy, dummy);
    isl::compute_flow(write, write, read, schedule, waw, war, dummy, dummy);

    isl::union_set domain = s.schedule.get_domain();

    isl::schedule_constraints sched_constraints(domain);

    isl::union_map validity = isl::union_(raw, isl::union_(waw, war));
    isl::union_map proximity = validity;

    sched_constraints.set_validity_constraints(validity);
    sched_constraints.set_proximity_constraints(proximity);
    sched_constraints.set_coincidence_constraints(validity);

    isl_options_set_schedule_fuse(isl_ctx.native_handle(), ISL_SCHEDULE_FUSE_MIN);
    isl_options_set_schedule_maximize_band_depth(isl_ctx.native_handle(), 1);
    isl_options_set_schedule_max_coefficient(isl_ctx.native_handle(), 20);
    isl_options_set_schedule_max_constant_term(isl_ctx.native_handle(), 20);

    isl::schedule sched(sched_constraints);

    s.schedule = get_schedule(get_optimized_schedule_tree(sched.get_root(), s));

    //s.schedule.dump();

    return s;
}

expression isl_ast_expr_to_kir(const isl::ast_expr& expr,
                               const std::map<std::string, expression>& symbol_table)
{
    using pattern::_;

    switch (expr.type())
    {
    case isl_ast_expr_id:
        return symbol_table.at(expr.get_id().name());
    case isl_ast_expr_int:
        return integer_literal_expr(expr.get_value().as_integer());
    case isl_ast_expr_op:
        switch (expr.get_op_type())
        {
        case isl_ast_op_call:
        {
            // TODO: assert that the left-most child is an id

            std::vector<expression> indices;

            for (int i = 1; i < expr.arg_count(); ++i)
            {
                indices.push_back(isl_ast_expr_to_kir(expr.get_arg(i), symbol_table));
            }

            try
            {
                auto macro = symbol_table.at(expr.get_arg(0).get_id().name()).as<macro_expr>();

                return expand_macro(macro, indices);
            }
            catch (const std::bad_cast&)
            {
                throw 0;
            }
        }
        case isl_ast_op_add:
            return binary_operator_expr(binary_op_tag::plus,
                                        isl_ast_expr_to_kir(expr.get_arg(0), symbol_table),
                                        isl_ast_expr_to_kir(expr.get_arg(1), symbol_table));
        case isl_ast_op_sub:
            return binary_operator_expr(binary_op_tag::minus,
                                        isl_ast_expr_to_kir(expr.get_arg(0), symbol_table),
                                        isl_ast_expr_to_kir(expr.get_arg(1), symbol_table));
        case isl_ast_op_mul:
            return binary_operator_expr(binary_op_tag::multiplies,
                                        isl_ast_expr_to_kir(expr.get_arg(0), symbol_table),
                                        isl_ast_expr_to_kir(expr.get_arg(1), symbol_table));
        case isl_ast_op_div:
            return binary_operator_expr(binary_op_tag::divides,
                                        isl_ast_expr_to_kir(expr.get_arg(0), symbol_table),
                                        isl_ast_expr_to_kir(expr.get_arg(1), symbol_table));
        case isl_ast_op_fdiv_q:
            return binary_operator_expr(binary_op_tag::div_floor,
                                        isl_ast_expr_to_kir(expr.get_arg(0), symbol_table),
                                        isl_ast_expr_to_kir(expr.get_arg(1), symbol_table));
        case isl_ast_op_pdiv_q:
            return binary_operator_expr(binary_op_tag::divides,
                                        isl_ast_expr_to_kir(expr.get_arg(0), symbol_table),
                                        isl_ast_expr_to_kir(expr.get_arg(1), symbol_table));
        case isl_ast_op_pdiv_r:
            return binary_operator_expr(binary_op_tag::modulus,
                                        isl_ast_expr_to_kir(expr.get_arg(0), symbol_table),
                                        isl_ast_expr_to_kir(expr.get_arg(1), symbol_table));
        case isl_ast_op_minus:
            return unary_operator_expr(unary_op_tag::negate,
                                       isl_ast_expr_to_kir(expr.get_arg(0), symbol_table));
        case isl_ast_op_min:
            return intrinsic_function_expr("min",
                                           {isl_ast_expr_to_kir(expr.get_arg(0), symbol_table),
                                            isl_ast_expr_to_kir(expr.get_arg(1), symbol_table)});
        case isl_ast_op_max:
            return intrinsic_function_expr("max",
                                           {isl_ast_expr_to_kir(expr.get_arg(0), symbol_table),
                                            isl_ast_expr_to_kir(expr.get_arg(1), symbol_table)});
        default:
            throw 0; // TODO: unknown op type
        }
    default:
        throw 0; // TODO: unknown expr type
    }
}

expression isl_ast_to_kir(const isl::ast_node& root,
                          std::map<std::string, expression>& symbol_table)
{
    static long int current_extracted_fn_id = 0;

    switch (root.type())
    {
    case isl_ast_node_for:
    {
        // Assume that a for loop is of the form
        // for (int i = init; i </<= upper_bound; i += inc)

        auto iterator = root.for_get_iterator();

        if (iterator.type() != isl_ast_expr_id)
            throw 0; // iterator has to be an id expr (might want to use an assert)

        // TODO: add new index to the lookup table
        variable_declaration idx_decl(types::integer{});

        symbol_table.emplace(iterator.get_id().name(), variable_ref_expr(idx_decl));

        auto lower_bound = isl_ast_expr_to_kir(root.for_get_init(), symbol_table);
        auto increment = isl_ast_expr_to_kir(root.for_get_inc(), symbol_table);

        auto cond = root.for_get_cond();

        auto upper_bound = [&]
        {
            // Assume that the cond is of the form
            // i </<= expr
            // where i is the current iterator.

            // TODO: assert if cond is not an op

            switch (cond.get_op_type())
            {
            case isl_ast_op_le:
            {
                auto rhs = isl_ast_expr_to_kir(cond.get_arg(1), symbol_table);

                return expression(
                    binary_operator_expr(binary_op_tag::plus, rhs, integer_literal_expr(1)));
            }
            case isl_ast_op_lt:
                return isl_ast_expr_to_kir(cond.get_arg(1), symbol_table);
            default:
                throw 0; // unexpected expr in cond
            }
        }();

        auto body = isl_ast_to_kir(root.for_get_body(), symbol_table);

        symbol_table.erase(iterator.get_id().name());

        return for_expr(idx_decl, lower_bound, upper_bound, increment, body);
    }
    case isl_ast_node_block:
    {
        std::vector<expression> sub_exprs;

        for (const auto& child : root.block_get_children())
        {
            sub_exprs.emplace_back(isl_ast_to_kir(child, symbol_table));
        }

        return compound_expr(sub_exprs);
    }
    case isl_ast_node_user:
    {
        return isl_ast_expr_to_kir(root.user_get_expr(), symbol_table);
    }
    case isl_ast_node_if:
        throw 0; // TODO: currently not supported
    case isl_ast_node_mark:
    {
        auto mark_id = root.mark_get_id();
        
        if (mark_id.name() == "task")
        {
            auto body = isl_ast_to_kir(root.mark_get_node(), symbol_table);

            return extract_expr_as_function(body, "extracted_fn_" + std::to_string(current_extracted_fn_id++));
        }
        
        return isl_ast_to_kir(root.mark_get_node(), symbol_table);
    }
    default:
        throw 0; // TODO: Unknown node type
    }
}

expression generate_code_from_scop(const scop& s)
{
    /*auto constr_map1 = isl::union_map(isl_ctx, "[A, C] -> { [i0, i1, i2, i3] -> [0,i0, i1, i2, i3]
    : i0 + 300 < A and i1 + 300 < C }");

    auto comp = isl::set(isl_set_complement(isl::set(isl_ctx, "[A, C] -> { [i0, i1, i2, i3] : i0 +
    300 < A and i1 + 300 < C }").release()));

    auto constr_map2 = isl::union_map(isl_ctx, "[A, C] -> { [i0, i1, i2, i3] -> [1,i0, i1, i2, i3] :
    i0 >= -300 + A or (i0 <= -301 + A and i1 >= -300 + C) }");

    isl_set_dump(comp.native_handle());

    auto new_schedule_full =
    isl::union_map(isl_union_map_apply_range(isl::union_map(new_schedule).release(),
    constr_map1.release()));
    auto new_schedule_partial =
    isl::union_map(isl_union_map_apply_range(isl::union_map(new_schedule).release(),
    constr_map2.release()));

    new_schedule = union_(new_schedule_full, new_schedule_partial);*/

    isl_options_set_ast_build_atomic_upper_bound(isl_ctx.native_handle(), 1);

    auto options =
        isl::union_map(isl_ctx, "[A, C] -> { [i0, i1, i2, i3] -> separation_class[[0]->[0]] : 0 <= "
                                "i0 + 300 and 0 <= i1 + 300 and i0 + 300 < A and i1 + 300 < C ;"
                                "[i0, i1, i2, i3] -> separation_class[[1]->[0]] : 0 <= i0 + 300 "
                                "and 0 <= i1 + 300 and i0 + 300 < A and i1 + 300 < C}");

    isl::ast_builder builder(isl_ctx);

    // builder.set_options(options);

    /*isl_union_map_dump(new_schedule.native_handle());

    isl::union_set domain(isl_ctx, "[N] -> { I[i,j,k] : 0 < i,j,k <= N; J[i,j,k] : 0 < i,j,k <=
    N}");

    isl::basic_map access1(isl_ctx, "[N] -> { I[i,j,k] -> C[i,j] : 0 < i,j,k <= N }");
    isl::basic_map access2(isl_ctx, "[N] -> { J[i,j,k] -> C[i,j] : 0 < i,j,k <= N }");

    isl::union_map sched(isl_ctx, "[N] -> { I[i,j,k] -> [1,i,j,k] : exists e0 = floor((j)/2) : j =
    2e0 and 0 < i,j,k <= N ; J[i,j,k] -> [2,i,j,k] : exists e0 = floor((j)/2) : j = 2e0 and 0 <
    i,j,k <= N}");

    auto transformed_access1 = apply_domain(access1, sched);
    auto transformed_access2 = apply_domain(access2, sched);

    auto time_domain = apply(domain, sched);

    isl_union_set_dump(time_domain.native_handle());

    isl_union_map_dump(transformed_access1.native_handle());
    isl_union_map_dump(transformed_access2.native_handle());

    auto accesses1 = apply(time_domain, transformed_access1);
    auto accesses2 = apply(time_domain, transformed_access2);

    isl_union_set_dump(accesses1.native_handle());
    isl_union_set_dump(accesses2.native_handle());

    auto aliases = intersect(accesses1, accesses2);

    isl_union_set_dump(aliases.native_handle());*/

    /*builder.set_before_each_for([&](isl::ast_builder_ref QBB_UNUSED(builder))
                                {
                                    return isl::id(isl_ctx, "test");
                                });*/

    isl::printer printer(isl_ctx);

    auto ast = builder.build_node_from_schedule(s.schedule);

    //printer.print(ast);

    auto symbol_table = s.symbol_table;

    return isl_ast_to_kir(ast, symbol_table);
}

expression detect_and_optimize_scops(expression expr)
{
    if (is_scop(expr))
    {
        expr = lower_abstract_indices(expr);

        auto optimized_scop = optimize_scop(analyze_scop(expr));

        return generate_code_from_scop(optimized_scop);
    }
    else
    {
        // TODO: recurse into children
        return expr;
    }
}
}

function_declaration optimize_loops(function_declaration decl)
{
    auto new_body = detect_and_optimize_scops(decl.body());

    decl.substitute_body(std::move(new_body));

    return decl;
}
}
}

