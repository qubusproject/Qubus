#include <qbb/qubus/loop_optimizer.hpp>

#include <qbb/qubus/lower_abstract_indices.hpp>

#include <qbb/qubus/IR/pretty_printer.hpp>

#include <qbb/qubus/IR/extract.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <qbb/qubus/isl/context.hpp>
#include <qbb/qubus/isl/set.hpp>
#include <qbb/qubus/isl/map.hpp>
#include <qbb/qubus/isl/pw_multi_aff.hpp>
#include <qbb/qubus/isl/constraint.hpp>
#include <qbb/qubus/isl/multi_union_pw_affine_expr.hpp>
#include <qbb/qubus/isl/flow.hpp>
#include <qbb/qubus/isl/schedule.hpp>
#include <qbb/qubus/isl/ast_builder.hpp>

#include <qbb/util/unique_name_generator.hpp>
#include <qbb/util/integers.hpp>
#include <qbb/util/numeric/bisection.hpp>
#include <qbb/util/numeric/polynomial.hpp>
#include <qbb/util/unused.hpp>

#include <boost/variant.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <vector>
#include <map>
#include <string>
#include <tuple>
#include <cassert>

#include <qbb/qubus/IR/pretty_printer.hpp>
#include <iostream>
#include <qbb/qubus/isl/printer.hpp>

namespace qbb
{
namespace qubus
{

namespace
{

isl::context isl_ctx;

struct array_substitution
{
    array_substitution(variable_declaration parent, macro_expr substitution)
    : parent(std::move(parent)), substitution(std::move(substitution))
    {
    }

    variable_declaration parent;
    macro_expr substitution;
};

struct ast_converter_context
{
    explicit ast_converter_context(std::map<std::string, expression> symbol_table)
    : symbol_table(std::move(symbol_table)), array_subs_scopes(1)
    {
    }

    std::map<std::string, expression> symbol_table;
    std::vector<array_substitution> array_substitutions;
    std::vector<std::vector<variable_declaration>> array_subs_scopes;
};

expression isl_ast_expr_to_kir(const isl::ast_expr& expr, ast_converter_context& ctx);

expression isl_ast_to_kir(const isl::ast_node& root, ast_converter_context& ctx);

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
         std::map<std::string, expression> symbol_table,
         std::map<std::string, variable_declaration> tensor_table)
    : schedule(isl::schedule::from_domain(domain)), write_accesses(write_accesses),
      read_accesses(read_accesses), symbol_table(symbol_table), tensor_table(tensor_table)
    {
        auto part_sched =
            isl::multi_union_pw_affine_expr::from_union_map(pad_schedule_map(schedule));

        isl::schedule_node root = this->schedule.get_root();

        auto new_ = insert_partial_schedule(root[0], part_sched);

        this->schedule = get_schedule(new_);
    }

    std::string add_symbol(const std::string& id, expression expr)
    {
        std::string unique_id = id;

        auto iter = symbol_table.find(unique_id);

        long int i = 1;
        while (iter != symbol_table.end())
        {
            unique_id = id + std::to_string(i);

            ++i;
            iter = symbol_table.find(unique_id);
        }

        symbol_table.emplace(unique_id, expr);

        return unique_id;
    }

    isl::schedule schedule;

    isl::union_map write_accesses;
    isl::union_map read_accesses;

    std::map<std::string, expression> symbol_table;
    std::map<std::string, variable_declaration> tensor_table;
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

    const std::string& map_tensor_to_name(variable_declaration id) const
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
        std::map<std::string, variable_declaration> reverse_tensor_table;

        for (const auto& entry : tensor_table)
        {
            reverse_tensor_table.emplace(entry.second, entry.first);
        }

        return scop(domain, param_constraints, schedule, write_accesses, read_accesses,
                    symbol_table, reverse_tensor_table);
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

    mutable std::map<variable_declaration, std::string> tensor_table;
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

bool is_scop(const expression& expr)
{
    auto m = pattern::make_matcher<expression, bool>().case_(pattern::sparse_tensor(), []
                                                             {
                                                                 return true;
                                                             });

    auto result = pattern::search(expr, m);

    return !result;
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
            auto tensor_name = ctx.map_tensor_to_name(decl.get());

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
    // TODO: Index bounds

    // TODO: Assert pos >= 0.

    auto n_dim = m.dim(type);

    // TODO: Assert pos + n < n_dim.

    isl::space s(m.get_ctx(), n, n_dim, n_dim - n);

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
                     .set_coefficient(isl_dim_out, i - (pos + n), -1);

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
        throw 0; // TODO: Throw exception.
    }
}

class array_tile
{
public:
    array_tile(variable_declaration parent, std::vector<isl::pw_aff> origin,
               std::vector<isl::pw_aff> shape, isl::map access_schedule, isl::map layout_transform,
               bool is_mutable)
    : parent(std::move(parent)), origin(std::move(origin)), shape(std::move(shape)),
      access_schedule(std::move(access_schedule)), layout_transform(std::move(layout_transform)),
      is_mutable(is_mutable)
    {
    }

    variable_declaration parent;
    std::vector<isl::pw_aff> origin;
    std::vector<isl::pw_aff> shape;
    isl::map access_schedule;
    isl::map layout_transform;
    bool is_mutable;
};

boost::optional<array_tile> deduce_tile(isl::map write_schedule, isl::map local_access_schedule,
                                        bool is_mutable, const scop& s)
{
    auto parametrized_write_schedule =
        turn_dims_into_params(write_schedule, isl_dim_in, 0, write_schedule.dim(isl_dim_in));

    for (int i = 0; i < write_schedule.dim(isl_dim_in); ++i)
    {
        auto outer_dim_name = "outer_dim" + std::to_string(i);

        write_schedule.set_dim_id(isl_dim_in, i, isl::id(isl_ctx, outer_dim_name));
        parametrized_write_schedule.set_dim_id(isl_dim_param, i + write_schedule.dim(isl_dim_param),
                                               isl::id(isl_ctx, outer_dim_name));
    }

    auto iter_space_dim = local_access_schedule.dim(isl_dim_in);

    auto current_to_next_iter =
        isl::map::universe(isl::space(isl_ctx, 0, iter_space_dim, iter_space_dim));

    for (int i = 0; i < iter_space_dim; ++i)
    {
        auto c = isl::constraint::equality(current_to_next_iter.get_space())
                     .set_coefficient(isl_dim_in, i, -1)
                     .set_coefficient(isl_dim_out, i, 1)
                     .set_constant(i == iter_space_dim - 1 ? 1 : 0);

        current_to_next_iter.add_constraint(c);
    }

    auto next_accesses = apply_domain(local_access_schedule, current_to_next_iter);

    auto proximity = apply_domain(local_access_schedule, next_accesses);

    auto domain = proximity.domain();

    isl::schedule_constraints constraints(domain);
    constraints.set_proximity_constraints(proximity);

    isl::schedule layout_schedule(constraints);

    auto layout_transform = layout_schedule.get_map().get_maps()[0];
    layout_transform.set_tuple_name(isl_dim_out,
                                    write_schedule.get_tuple_name(isl_dim_out) + "_scratch");

    auto new_layout = apply(parametrized_write_schedule.range(), layout_transform);

    auto dim = new_layout.dim(isl_dim_set);

    std::vector<isl::pw_aff> origin;
    std::vector<isl::pw_aff> shape;

    for (int i = 0; i < dim; ++i)
    {
        auto red_set = project_out(project_out(new_layout, isl_dim_set, 0, i), isl_dim_set, i + 1,
                                   dim - (i + 1));

        if (red_set.bounded())
        {
            auto max = lexmax_pw_multi_aff(red_set)[0];
            auto min = lexmin_pw_multi_aff(red_set)[0];

            auto diff = max - min;

            auto one = isl::pw_aff::from_val(diff.domain(), isl::value(isl_ctx, 1));

            auto local_sizes = set_from_pw_aff(diff + one);

            auto global_sizes =
                project_out(local_sizes, isl_dim_param, 0, local_sizes.dim(isl_dim_param));

            if (global_sizes.bounded())
            {
                auto size = lexmax_pw_multi_aff(global_sizes)[0];

                if (size.is_cst())
                {
                    origin.push_back(min);
                    shape.push_back(size);
                }
                else
                {
                    return boost::none;
                }
            }
            else
            {
                return boost::none;
            }
        }
        else
        {
            return boost::none;
        }
    }

    // TODO: shift new_layout to (0,0,...)
    boost::optional<isl::map> offsets;

    for (const auto offset : origin)
    {
        isl::space s(isl_ctx, 0, 1, 1);

        auto ident = isl::pw_multi_aff::from_map(
            align_params(isl::map::identity(s), offset.domain().get_space()))[0];
        auto offset2 = isl::pw_multi_aff::from_map(
            align_params(intersect_range(isl::map::universe(s), set_from_pw_aff(offset)),
                         offset.domain().get_space()))[0];

        auto pos = ident - offset2;

        auto shift_transform = isl::map(isl_map_from_pw_aff(pos.release()));

        shift_transform = detect_equalities(std::move(shift_transform));
        shift_transform = remove_redundancies(std::move(shift_transform));
        shift_transform = coalesce(std::move(shift_transform));

        if (offsets)
        {
            offsets = flat_product(*offsets, shift_transform);
        }
        else
        {
            offsets = shift_transform;
        }
    }

    offsets->set_tuple_name(isl_dim_in, layout_transform.get_tuple_name(isl_dim_out));
    offsets->set_tuple_name(isl_dim_out, layout_transform.get_tuple_name(isl_dim_out));

    layout_transform = apply_range(layout_transform, *offsets);

    if (!layout_transform.is_injective())
    {
        return boost::none;
    }

    const auto& array_id = s.tensor_table.at(write_schedule.get_tuple_name(isl_dim_out));

    return array_tile(array_id, origin, shape, write_schedule, layout_transform, is_mutable);
}

std::vector<array_tile> deduce_tiles(isl::schedule_node node, const scop& s)
{
    auto prefix_schedule = node.get_prefix_schedule_union_map();

    auto subtree_schedule = node.get_subtree_schedule_union_map();

    auto mutating_accesses = s.write_accesses;
    auto read_only_accesses = substract(s.read_accesses, s.write_accesses);

    auto write_schedule = apply_domain(mutating_accesses, prefix_schedule);
    auto local_write_schedule = apply_domain(mutating_accesses, subtree_schedule);

    std::vector<array_tile> tiles;

    for (const auto& schedule : write_schedule.get_maps())
    {
        auto local_writes_schedules = local_write_schedule.get_maps();

        auto iter = boost::find_if(local_writes_schedules, [&](const isl::map& value)
                                   {
                                       return value.get_tuple_name(isl_dim_out) ==
                                              schedule.get_tuple_name(isl_dim_out);
                                   });

        if (iter == local_writes_schedules.end())
            throw 0;

        if (auto tile = deduce_tile(schedule, *iter, true, s))
        {
            tiles.push_back(*tile);
        }
    }

    auto read_schedule = apply_domain(read_only_accesses, prefix_schedule);
    auto local_read_schedule = apply_domain(read_only_accesses, subtree_schedule);

    for (const auto& schedule : read_schedule.get_maps())
    {
        auto local_read_schedules = local_read_schedule.get_maps();

        auto iter = boost::find_if(local_read_schedules, [&](const isl::map& value)
                                   {
                                       return value.get_tuple_name(isl_dim_out) ==
                                              schedule.get_tuple_name(isl_dim_out);
                                   });

        if (iter == local_read_schedules.end())
            throw 0;

        if (auto tile = deduce_tile(schedule, *iter, false, s))
        {
            tiles.push_back(*tile);
        }
    }

    return tiles;
}

enum class cache_copy_direction
{
    mem_to_cache,
    cache_to_mem
};

isl::schedule_node create_cache_copy(const array_tile& tile, variable_declaration cache_decl,
                                     cache_copy_direction direction, bool unroll_loops, scop& s)
{
    isl::ast_builder builder(isl_ctx);

    const auto& map = tile.access_schedule;

    auto dim = map.dim(isl_dim_in) + map.dim(isl_dim_out);

    isl::space ms(isl_ctx, 0, map.dim(isl_dim_out), dim);
    auto m = isl::basic_map::universe(ms);

    for (int i = map.dim(isl_dim_in); i < dim; ++i)
    {
        auto c = isl::constraint::equality(ms)
                     .set_coefficient(isl_dim_in, i - map.dim(isl_dim_in), -1)
                     .set_coefficient(isl_dim_out, i, 1);

        m.add_constraint(c);
    }

    std::vector<variable_declaration> params;

    for (int i = 0; i < dim; ++i)
    {
        params.emplace_back(types::integer());
    }

    auto local_symbol_table = s.symbol_table;

    for (int i = 0; i < tile.access_schedule.dim(isl_dim_in); ++i)
    {
        local_symbol_table.emplace(tile.access_schedule.get_dim_name(isl_dim_in, i),
                                   variable_ref_expr(params[i]));
    }

    for (int i = map.dim(isl_dim_in); i < dim; ++i)
    {
        local_symbol_table.emplace("c" + std::to_string(i - map.dim(isl_dim_in)),
                                   variable_ref_expr(params[i]));
    }

    isl::set dom = tile.access_schedule.range();
    isl::ast_builder builder2(dom);

    isl::map mm = isl::map::identity(tile.layout_transform.get_space());
    mm.set_tuple_name(isl_dim_in, dom.get_tuple_name());
    mm.set_tuple_name(isl_dim_out, dom.get_tuple_name());

    isl::map access1 = intersect_domain(mm, dom);
    isl::map access2 = intersect_domain(tile.layout_transform, dom);

    auto parent_access =
        builder2.build_access_from_pw_multi_aff(isl::pw_multi_aff::from_map(access1));
    auto scratch_access =
        builder2.build_access_from_pw_multi_aff(isl::pw_multi_aff::from_map(access2));

    local_symbol_table.emplace(dom.get_tuple_name(), variable_ref_expr(tile.parent));
    local_symbol_table.emplace(access2.get_tuple_name(isl_dim_out), variable_ref_expr(cache_decl));

    ast_converter_context ctx(local_symbol_table);

    auto global_access = isl_ast_expr_to_kir(parent_access, ctx);
    auto local_access = isl_ast_expr_to_kir(scratch_access, ctx);

    auto copy_code = [&]
    {
        if (direction == cache_copy_direction::mem_to_cache)
        {
            auto lhs = local_access;
            auto rhs = global_access;

            return binary_operator_expr(binary_op_tag::assign, lhs, rhs);
        }
        else if (direction == cache_copy_direction::cache_to_mem)
        {
            auto lhs = global_access;
            auto rhs = local_access;

            return binary_operator_expr(binary_op_tag::assign, lhs, rhs);
        }
        else
        {
            throw 0;
        }
    }();

    auto copy_id = s.add_symbol("copy", macro_expr(params, copy_code));

    m.set_tuple_name(isl_dim_out, copy_id);
    m.set_tuple_name(isl_dim_in, map.get_tuple_name(isl_dim_out));

    auto extension = apply_range(map, m);

    for (int i = 0; i < map.dim(isl_dim_in); ++i)
    {
        auto c = isl::constraint::equality(extension.get_space())
                     .set_coefficient(isl_dim_in, i, -1)
                     .set_coefficient(isl_dim_out, i, 1);

        extension.add_constraint(c);
    }

    auto prefix_dim = extension.dim(isl_dim_in);

    auto local_dim = extension.dim(isl_dim_out) - prefix_dim;

    isl::space local_iter_space(isl_ctx, 0, local_dim);
    auto s2 = isl::basic_set::universe(local_iter_space);

    auto copy_schedule = make_map_from_domain_and_range(extension.range(), s2);

    for (int i = 0; i < local_dim; ++i)
    {
        auto c = isl::constraint::equality(copy_schedule.get_space())
                     .set_coefficient(isl_dim_in, i + prefix_dim, -1)
                     .set_coefficient(isl_dim_out, i, 1);

        copy_schedule.add_constraint(c);
    }

    auto extension_root = isl::schedule_node::from_extension(extension);

    auto copy_band = isl::insert_partial_schedule(
        extension_root[0], isl::multi_union_pw_affine_expr::from_union_map(copy_schedule));

    if (unroll_loops)
    {
        for (int i = 0; i < local_dim; ++i)
        {
            copy_band.band_member_set_ast_loop_type(i, isl_ast_loop_unroll);
        }
    }

    extension_root = copy_band.parent();

    return extension_root;
}

struct cache_info
{
    cache_info(variable_declaration parent, macro_expr substitution)
    : parent(std::move(parent)), substitution(std::move(substitution))
    {
    }

    variable_declaration parent;
    macro_expr substitution;
};

isl::schedule_node create_caches(isl::schedule_node band_to_tile, bool unroll_loops, scop& s)
{
    isl::ast_builder builder(isl_ctx);

    auto tiles = deduce_tiles(band_to_tile, s);

    auto prefix_schedule = band_to_tile.get_prefix_schedule_union_map();

    auto subtree_schedule = band_to_tile.get_subtree_schedule_union_map();

    auto prefix_range = prefix_schedule.range().get_sets();

    if (prefix_range.size() != 1)
        throw 0;

    auto outer_dims = prefix_range[0];

    auto outer_dim = outer_dims.dim(isl_dim_set);

    for (const auto& tile : tiles)
    {
        std::vector<variable_declaration> constr_params;

        for (int i = 0; i < outer_dim; ++i)
        {
            constr_params.emplace_back(types::integer());
        }

        variable_declaration cache_decl(tile.parent.var_type());

        std::vector<expression> constr_args;

        for (const auto& extent : tile.shape)
        {
            ast_converter_context ctx({});

            auto expr = builder.build_expr_from_pw_aff(extent);

            constr_args.push_back(isl_ast_expr_to_kir(expr, ctx));
        }

        auto local_symbol_table = s.symbol_table;

        for (int i = 0; i < outer_dim; ++i)
        {
            local_symbol_table.emplace(tile.access_schedule.get_dim_name(isl_dim_in, i),
                                       variable_ref_expr(constr_params[i]));
        }

        expression cache_constr_code = local_variable_def_expr(
            cache_decl, construct_expr(tile.parent.var_type(), constr_args));

        isl::ast_builder transform_builder(tile.layout_transform.domain());

        auto transform_ast = transform_builder.build_access_from_pw_multi_aff(
            isl::pw_multi_aff::from_map(tile.layout_transform));

        auto num_inner_indices = tile.layout_transform.dim(isl_dim_in);

        std::vector<variable_declaration> inner_indices;

        for (int i = 0; i < num_inner_indices; ++i)
        {
            auto inner_index = variable_declaration(types::integer());

            inner_indices.push_back(inner_index);

            local_symbol_table.emplace("c" + std::to_string(i), variable_ref_expr(inner_index));
        }

        local_symbol_table.emplace(tile.layout_transform.get_tuple_name(isl_dim_out),
                                   variable_ref_expr(cache_decl));

        ast_converter_context ctx(local_symbol_table);

        auto transform_code = isl_ast_expr_to_kir(transform_ast, ctx);

        auto substitution = macro_expr(constr_params, macro_expr(inner_indices, transform_code));

        cache_info info(tile.parent, substitution);

        expression cache_constructor_code = macro_expr(constr_params, cache_constr_code);
        cache_constructor_code.annotations().add("qubus.cache_info", annotation(info));

        auto cache_constr_id = s.add_symbol("qubus.construct_cache", cache_constructor_code);

        isl::basic_map cache_constr_map =
            isl::basic_map::identity(isl::space(isl_ctx, 0, outer_dim, outer_dim));
        cache_constr_map.set_tuple_name(isl_dim_out, cache_constr_id);

        auto construct_extension = intersect_domain(cache_constr_map, outer_dims);

        band_to_tile =
            graft_before(band_to_tile, isl::schedule_node::from_extension(construct_extension));

        band_to_tile = graft_before(
            band_to_tile, create_cache_copy(tile, cache_decl, cache_copy_direction::mem_to_cache,
                                            unroll_loops, s));

        if (tile.is_mutable)
        {
            band_to_tile =
                graft_after(band_to_tile,
                            create_cache_copy(tile, cache_decl, cache_copy_direction::cache_to_mem,
                                              unroll_loops, s));
        }
    }

    return band_to_tile;
}

void separate_full_from_partial_tiles(isl::schedule_node& band, util::index_t top_level_tile_size)
{
    auto prefix_schedule = band.get_prefix_schedule_union_map();

    auto n_outer_dim = prefix_schedule.range().get_sets()[0].dim(isl_dim_set);

    auto subtree_schedule = band.get_subtree_schedule_union_map();

    auto n_member = band.band_n_member();

    isl::space s(isl_ctx, 0, n_member, n_member);
    auto m = isl::map::universe(s);

    for (int j = 0; j < n_member; ++j)
    {
        auto c = isl::constraint::equality(s)
                     .set_coefficient(isl_dim_in, j, -1)
                     .set_coefficient(isl_dim_out, j, 1)
                     .set_constant(top_level_tile_size - 1);

        m.add_constraint(c);
    }

    auto isolate_cond = isl::set::empty(isl::space(isl_ctx, 0, n_member));

    for (const auto& map : subtree_schedule.get_maps())
    {
        auto dom = extract_set(band.get_domain(), map.domain().get_space());

        auto time_dom = apply(dom, map);

        auto reduced_time_dom =
            project_out(time_dom, isl_dim_set, 0, time_dom.dim(isl_dim_set) - n_member);

        auto isolate_cond_ = apply(reduced_time_dom, m);

        isolate_cond = union_(isolate_cond, isolate_cond_);
    }

    auto outer_dims = flat_product(
        isolate_cond, isl::set::universe(isl::space(isl_ctx, 0, n_outer_dim - n_member)));

    auto inner_dims = isl::set::universe(isl::space(isl_ctx, 0, n_member));

    auto option = wrap(make_map_from_domain_and_range(outer_dims, inner_dims));
    option.set_tuple_name("isolate");

    band.band_set_ast_build_options(option);
}

isl::schedule_node optimize_schedule_node(isl::schedule_node root, scop& s)
{
    if (root.get_type() == isl_schedule_node_band)
    {
        if (root.band_is_permutable())
        {
            if (root[0].get_type() == isl_schedule_node_leaf)
            {
                isl_options_set_tile_shift_point_loops(isl_ctx.native_handle(), 0);
                isl_options_set_tile_scale_tile_loops(isl_ctx.native_handle(), 1);

                if (root.band_n_member() > 1)
                {
                    auto subtree_schedule = root.get_subtree_schedule_union_map();

                    auto local_writes = apply_domain(s.write_accesses, subtree_schedule);
                    auto local_reads = apply_domain(s.read_accesses, subtree_schedule);

                    auto local_accesses = union_(local_writes, local_reads);

                    bool no_reuse = local_accesses.is_injective();

                    if (!no_reuse)
                    {
                        std::vector<util::index_t> tile_sizes = {100, 20, 4};
                        std::size_t num_tile_levels = tile_sizes.size();

                        isl::schedule_node band_to_tile = root;

                        for (std::size_t i = 0; i < num_tile_levels; ++i)
                        {
                            std::vector<util::index_t> tile_shape(root.band_n_member(),
                                                                  tile_sizes[i]);

                            auto the_tile_band = tile_band(band_to_tile, tile_shape);

                            if (i == 1)
                            {
                                separate_full_from_partial_tiles(the_tile_band, tile_sizes[0]);
                            }

                            bool unroll_loops = i == num_tile_levels - 1;

                            band_to_tile = the_tile_band[0];

                            if (unroll_loops)
                            {
                                for (int i = 0; i < root.band_n_member(); ++i)
                                {
                                    band_to_tile.band_member_set_ast_loop_type(i,
                                                                               isl_ast_loop_unroll);
                                }
                            }

                            band_to_tile = insert_mark(band_to_tile, isl::id(isl_ctx, "task"))[0];

                            if (i == num_tile_levels - 1)
                            {
                                band_to_tile = create_caches(band_to_tile, unroll_loops, s);
                            }
                        }

                        return band_to_tile;
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

    isl_options_set_schedule_serialize_sccs(isl_ctx.native_handle(), 1);
    isl_options_set_schedule_maximize_band_depth(isl_ctx.native_handle(), 1);
    isl_options_set_schedule_max_coefficient(isl_ctx.native_handle(), 20);
    isl_options_set_schedule_max_constant_term(isl_ctx.native_handle(), 20);

    isl::schedule sched(sched_constraints);

    s.schedule = map_schedule_node(sched, [&](isl::schedule_node node)
                                   {
                                       return optimize_schedule_node(node, s);
                                   });

    // s.schedule.dump();

    return s;
}

expression isl_ast_expr_to_kir(const isl::ast_expr& expr, ast_converter_context& ctx)
{
    using pattern::_;

    const auto symbol_table = ctx.symbol_table;

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
                indices.push_back(isl_ast_expr_to_kir(expr.get_arg(i), ctx));
            }

            try
            {
                auto id = expr.get_arg(0).get_id().name();
                auto macro = symbol_table.at(id).as<macro_expr>();

                if (boost::starts_with(id, "qubus.construct_cache"))
                {
                    auto cinfo = macro.annotations().lookup("qubus.cache_info").as<cache_info>();

                    auto substitution = expand_macro(cinfo.substitution, indices).as<macro_expr>();

                    ctx.array_substitutions.emplace_back(cinfo.parent, substitution);
                    ctx.array_subs_scopes.back().push_back(cinfo.parent);
                }

                if (boost::starts_with(id, "copy"))
                {
                    return expand_macro(macro, indices);
                }
                else
                {
                    auto code = expand_macro(macro, indices);

                    pattern::variable<std::vector<expression>> indices;

                    for (const auto& substitution : ctx.array_substitutions)
                    {
                        auto m = pattern::make_matcher<expression, expression>().case_(
                            subscription(variable_ref(pattern::value(substitution.parent)),
                                         indices),
                            [&]
                            {
                                return expand_macro(substitution.substitution, indices.get());
                            });

                        code = pattern::substitute(code, m);
                    }

                    return code;
                }
            }
            catch (const std::bad_cast&)
            {
                throw 0;
            }
        }
        case isl_ast_op_add:
            return binary_operator_expr(binary_op_tag::plus,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_sub:
            return binary_operator_expr(binary_op_tag::minus,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_mul:
            return binary_operator_expr(binary_op_tag::multiplies,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_div:
            return binary_operator_expr(binary_op_tag::divides,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_fdiv_q:
            return binary_operator_expr(binary_op_tag::div_floor,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_pdiv_q:
            return binary_operator_expr(binary_op_tag::divides,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_pdiv_r:
            return binary_operator_expr(binary_op_tag::modulus,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_minus:
            return unary_operator_expr(unary_op_tag::negate,
                                       isl_ast_expr_to_kir(expr.get_arg(0), ctx));
        case isl_ast_op_min:
            return intrinsic_function_expr("min", {isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                                   isl_ast_expr_to_kir(expr.get_arg(1), ctx)});
        case isl_ast_op_max:
            return intrinsic_function_expr("max", {isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                                   isl_ast_expr_to_kir(expr.get_arg(1), ctx)});
        case isl_ast_op_cond:
        case isl_ast_op_select:
            return intrinsic_function_expr("select", {isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                                      isl_ast_expr_to_kir(expr.get_arg(1), ctx),
                                                      isl_ast_expr_to_kir(expr.get_arg(2), ctx)});
        case isl_ast_op_eq:
            return binary_operator_expr(binary_op_tag::equal_to,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_le:
            return binary_operator_expr(binary_op_tag::less_equal,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_lt:
            return binary_operator_expr(binary_op_tag::less,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_ge:
            return binary_operator_expr(binary_op_tag::greater_equal,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_gt:
            return binary_operator_expr(binary_op_tag::greater,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_and:
            return binary_operator_expr(binary_op_tag::logical_and,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_or:
            return binary_operator_expr(binary_op_tag::logical_or,
                                        isl_ast_expr_to_kir(expr.get_arg(0), ctx),
                                        isl_ast_expr_to_kir(expr.get_arg(1), ctx));
        case isl_ast_op_access:
        {
            auto array = isl_ast_expr_to_kir(expr.get_arg(0), ctx);

            std::vector<expression> indices;

            auto argc = expr.arg_count();

            for (int i = 1; i < argc; ++i)
                indices.push_back(isl_ast_expr_to_kir(expr.get_arg(i), ctx));

            return subscription_expr(array, indices);
        }
        default:
            throw 0; // TODO: unknown op type
        };
    default:
        throw 0; // TODO: unknown expr type
    }
}

expression isl_ast_to_kir(const isl::ast_node& root, ast_converter_context& ctx)
{
    // static long int current_extracted_fn_id = 0;

    auto& symbol_table = ctx.symbol_table;

    switch (root.type())
    {
    case isl_ast_node_for:
    {
        // Assume that a for loop is of the form
        // for (int i = init; i </<= upper_bound; i += inc)

        ctx.array_subs_scopes.emplace_back();

        auto iterator = root.for_get_iterator();

        if (iterator.type() != isl_ast_expr_id)
            throw 0; // iterator has to be an id expr (might want to use an assert)

        // TODO: add new index to the lookup table
        variable_declaration idx_decl(types::integer{});

        symbol_table.emplace(iterator.get_id().name(), variable_ref_expr(idx_decl));

        auto lower_bound = isl_ast_expr_to_kir(root.for_get_init(), ctx);
        auto increment = isl_ast_expr_to_kir(root.for_get_inc(), ctx);

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
                auto rhs = isl_ast_expr_to_kir(cond.get_arg(1), ctx);

                return expression(
                    binary_operator_expr(binary_op_tag::plus, rhs, integer_literal_expr(1)));
            }
            case isl_ast_op_lt:
                return isl_ast_expr_to_kir(cond.get_arg(1), ctx);
            default:
                throw 0; // unexpected expr in cond
            }
        }();

        auto body = isl_ast_to_kir(root.for_get_body(), ctx);

        auto iter = std::remove_if(ctx.array_substitutions.begin(), ctx.array_substitutions.end(),
                                   [&](const array_substitution& value)
                                   {
                                       return std::any_of(ctx.array_subs_scopes.back().begin(),
                                                          ctx.array_subs_scopes.back().end(),
                                                          [&](const variable_declaration& decl)
                                                          {
                                                              return value.parent == decl;
                                                          });
                                   });

        ctx.array_substitutions.erase(iter, ctx.array_substitutions.end());

        ctx.array_subs_scopes.pop_back();

        symbol_table.erase(iterator.get_id().name());

        return for_expr(idx_decl, std::move(lower_bound), std::move(upper_bound), std::move(increment), std::move(body));
    }
    case isl_ast_node_block:
    {
        std::vector<expression> sub_exprs;

        for (const auto& child : root.block_get_children())
        {
            sub_exprs.emplace_back(isl_ast_to_kir(child, ctx));
        }

        return compound_expr(std::move(sub_exprs));
    }
    case isl_ast_node_user:
    {
        return isl_ast_expr_to_kir(root.user_get_expr(), ctx);
    }
    case isl_ast_node_if:
    {
        auto condition = isl_ast_expr_to_kir(root.if_get_cond(), ctx);

        auto then_branch = isl_ast_to_kir(root.if_get_then(), ctx);

        auto else_block = root.if_get_else();

        if (else_block)
        {
            auto else_branch = isl_ast_to_kir(*else_block, ctx);

            return if_expr(std::move(condition), std::move(then_branch), std::move(else_branch));
        }
        else
        {
            return if_expr(std::move(condition), std::move(then_branch));
        }
    };
    case isl_ast_node_mark:
    {
        auto mark_id = root.mark_get_id();

        // TODO: Reenable this after fixing the resulting performance problems.
        /*if (mark_id.name() == "task")
        {
            auto body = isl_ast_to_kir(root.mark_get_node(), ctx);

            return extract_expr_as_function(body, "extracted_fn_" +
                                                      std::to_string(current_extracted_fn_id++));
        }*/

        return isl_ast_to_kir(root.mark_get_node(), ctx);
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

    isl::ast_builder builder(isl_ctx);

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

    // printer.print(ast);

    ast_converter_context ctx(s.symbol_table);

    return isl_ast_to_kir(ast, ctx);
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
