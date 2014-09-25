#include <qbb/kubus/loop_optimizer.hpp>

#include <qbb/kubus/deduce_loop_bounds_pass.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/kubus/isl/context.hpp>
#include <qbb/kubus/isl/set.hpp>
#include <qbb/kubus/isl/map.hpp>
#include <qbb/kubus/isl/constraint.hpp>
#include <qbb/kubus/isl/flow.hpp>
#include <qbb/kubus/isl/schedule.hpp>
#include <qbb/kubus/isl/ast_builder.hpp>

#include <qbb/util/multi_method.hpp>
#include <qbb/util/unique_name_generator.hpp>

#include <vector>
#include <string>
#include <tuple>
#include <mutex>
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

struct scop_stmt
{
    isl::set domain;
    isl::map scattering;
    isl::union_map write_access;
    isl::union_map read_access;

    std::vector<std::tuple<std::string, expression>> parameter_table;

    expression body;
};

struct scop_context
{
    scop_context() : domain(isl::set::universe(isl::space(isl_ctx, 0, 0))), scatter_index{0}
    {
    }

    isl::set domain;

    std::vector<long int> scatter_index;

    std::vector<std::tuple<std::string, expression>> parameter_table;

    util::unique_name_generator name_gen;
};

void extract_parameters(const expression& expr, scop_context& ctx)
{
    if (auto extent = expr.try_as<intrinsic_function_expr>())
    {
        if(extent->name() == "extent")
        {
            std::string new_param_id = ctx.name_gen.get();
            ctx.parameter_table.emplace_back(new_param_id, expr);

            expr.annotations().add("kubus.scop.param_id", qbb::kubus::annotation(new_param_id));
        
        }
    }
}

bool is_scop(const expression& expr)
{
    return true;
}

qbb::util::sparse_multi_method<void(const qbb::util::virtual_<expression>&, scop_context,
                                    std::vector<scop_stmt>&)> analyze_scop_ = {};

void analyze_scop_for_expr(const for_expr& expr, scop_context ctx, std::vector<scop_stmt>& stmts)
{
    std::cout << "found loop" << std::endl;

    extract_parameters(expr.lower_bound(), ctx);
    extract_parameters(expr.upper_bound(), ctx);

    isl::space local_iteration_space(isl_ctx, ctx.parameter_table.size(), 1);

    for (std::size_t i = 0; i < ctx.parameter_table.size(); ++i)
    {
        local_iteration_space.set_dim_name(isl_dim_param, i, std::get<0>(ctx.parameter_table[i]));
    }

    local_iteration_space.set_dim_name(isl_dim_set, 0, expr.index().as<index_expr>().id());

    isl::set local_domain = isl::set::universe(local_iteration_space);

    auto lower_bound_constraint =
        isl::constraint::inequality(local_iteration_space).set_coefficient(isl_dim_set, 0, 1);
    auto upper_bound_constraint =
        isl::constraint::inequality(local_iteration_space)
            .set_coefficient(isl_dim_set, 0, -1)
            .set_coefficient(isl_dim_param, ctx.parameter_table.size() - 1, 1)
            .set_constant(-1);

    local_domain.add_constraint(lower_bound_constraint);
    local_domain.add_constraint(upper_bound_constraint);

    ctx.domain = flat_product(ctx.domain, local_domain);

    ctx.scatter_index.push_back(0);

    analyze_scop_(expr.body(), ctx, stmts);
}

void analyze_scop_for_all_expr(const for_all_expr&, scop_context, std::vector<scop_stmt>&)
{
    throw 0; // error: for all loops should be removed by now, TODO: add proper exception
}

void analyze_scop_compound_expr(const compound_expr& expr, scop_context ctx,
                                std::vector<scop_stmt>& stmts)
{
    std::cout << "found compound" << std::endl;

    for (const auto& sub_expr : expr.body())
    {
        analyze_scop_(sub_expr, ctx, stmts);
        ++ctx.scatter_index.back();
    }
}

void analyze_scop_default(const expression& expr, scop_context ctx, std::vector<scop_stmt>& stmts)
{
    std::cout << "we entered the loop body" << std::endl;
    
    // we entered the loop body
    
    isl_set_dump(ctx.domain.native_handle());

    isl::set domain = ctx.domain;
        
    domain.set_tuple_name(std::string("stmt") + std::to_string(stmts.size()));
    
    isl_space_dump(domain.get_space().native_handle());
    
    long int number_of_indices = domain.get_space().dim(isl_dim_set);
    long int number_of_dimensions = 2 * number_of_indices + 1;

    assert(number_of_indices + 1 == ctx.scatter_index.size());
    
    isl::set scattering_domain = isl::set::universe(drop_all_dims(domain.get_space(), isl_dim_param));
    
    isl::space scattering_range_space(isl_ctx, 0, number_of_dimensions);
    scattering_range_space.set_tuple_name(isl_dim_set, "scattering");
    
    isl::set scattering_range = isl::set::universe(scattering_range_space);
    
    isl::map scattering = make_map_from_domain_and_range(scattering_domain, scattering_range);
    
    scattering.add_constraint(isl::constraint::equality(scattering.get_space()).set_coefficient(isl_dim_out, 0, 1).set_constant(-ctx.scatter_index[0]));
    for(std::size_t  i = 0; i < number_of_indices; ++i)
    {
        scattering.add_constraint(isl::constraint::equality(scattering.get_space()).set_coefficient(isl_dim_out, 2*i + 2, 1).set_constant(-ctx.scatter_index[i+1]));
        scattering.add_constraint(isl::constraint::equality(scattering.get_space()).set_coefficient(isl_dim_out, 2*i + 1, 1).set_coefficient(isl_dim_in, i, -1));
    }
    
    isl_map_dump(scattering.native_handle());
    
    //analyze_tensor_accesses(expr, domain);
}

void init_analyze_scop()
{
    analyze_scop_.add_specialization(analyze_scop_for_expr);
    analyze_scop_.add_specialization(analyze_scop_for_all_expr);
    analyze_scop_.add_specialization(analyze_scop_compound_expr);

    analyze_scop_.set_fallback(analyze_scop_default);
}

std::once_flag analyze_scop_init_flag = {};

std::vector<scop_stmt> analyze_scop(const expression& expr)
{
    std::call_once(analyze_scop_init_flag, init_analyze_scop);

    std::vector<scop_stmt> stmts;

    scop_context ctx;

    analyze_scop_(expr, ctx, stmts);

    isl::map scattering0(isl_ctx, "{stmt0[i,j] -> scattering[0,i,0,j,0]}");
    isl::set domain0(isl_ctx, "[N,M] -> {stmt0[i,j] : 0 <= i < N and 0 <= j < M}");
    isl::union_map write_access0(isl_ctx,
                                 "[N,M] -> { stmt0[i,j] -> C[i,j] : 0 <= i < N and 0 <= j < M }");
    isl::union_map read_access0(isl_ctx, "[N,M] -> {}");

    stmts.push_back(
        scop_stmt{domain0, scattering0, write_access0, read_access0, {}, double_literal_expr(0.0)});

    isl::map scattering1(isl_ctx, "{stmt1[i,j,k] -> scattering[0,i,0,j,1,k,0]}");
    isl::set domain1(isl_ctx,
                     "[N,M,L] -> {stmt1[i,j,k] : 0 <= i < N and 0 <= j < M and 0 <= k < L}");
    isl::union_map write_access1(
        isl_ctx, "[N,M,L] -> {stmt1[i,j,k] -> C[i,j] : 0 <= i < N and 0 <= j < M and 0 <= k < L}");
    isl::union_map read_access1(
        isl_ctx, "[N,M,L] -> { stmt1[i,j,k] -> A[i,k] : 0 <= i < N and 0 <= j < M and 0 <= k < L ;"
                 "stmt1[i,j,k] -> B[k,j] : 0 <= i < N and 0 <= j < M and 0 <= k < L;"
                 "stmt1[i,j,k] -> C[i,j] : 0 <= i < N and 0 <= j < M and 0 <= k < L }");

    stmts.push_back(
        scop_stmt{domain1, scattering1, write_access1, read_access1, {}, double_literal_expr(1.0)});

    return stmts;
}

isl::union_map optimize_scop(const std::vector<scop_stmt>& stmts)
{
    isl::printer print(isl_ctx);

    isl::set params(isl_ctx, "[N,M,L] -> {:}");

    isl::union_map schedule = isl::union_map::empty(params.get_space());

    for (const auto& stmt : stmts)
    {
        schedule = isl::add_map(schedule, stmt.scattering);
    }

    print.print(schedule);
    std::cout << std::endl;

    isl::union_map read = isl::union_map::empty(params.get_space());
    isl::union_map write = isl::union_map::empty(params.get_space());

    for (const auto& stmt : stmts)
    {
        read = union_(read, stmt.read_access);
        write = union_(write, stmt.write_access);
    }

    isl::union_map may_write = isl::union_map::empty(params.get_space());

    isl::union_map dummy = isl::union_map::empty(params.get_space());

    isl::union_map raw = isl::union_map::empty(params.get_space());
    isl::union_map waw = isl::union_map::empty(params.get_space());
    isl::union_map war = isl::union_map::empty(params.get_space());

    isl::compute_flow(read, write, may_write, schedule, raw, dummy, dummy, dummy);
    isl::compute_flow(write, write, read, schedule, waw, war, dummy, dummy);

    print.print(raw);
    std::cout << std::endl;
    print.print(waw);
    std::cout << std::endl;
    print.print(war);
    std::cout << std::endl;
    std::cout << std::endl;

    isl::union_set domain = isl::union_set::empty(params.get_space());

    for (const auto& stmt : stmts)
    {
        domain = union_(domain, stmt.domain);
    }

    isl::schedule_constraints sched_constraints(domain);

    isl::union_map validity = isl::union_(raw, isl::union_(waw, war));
    isl::union_map proximity = validity;

    sched_constraints.set_validity_constraints(validity);
    sched_constraints.set_proximity_constraints(proximity);
    sched_constraints.set_coincidence_constraints(validity);

    isl::schedule sched(sched_constraints);

    sched.dump();

    auto new_schedule = isl::intersect_domain(sched.get_map(), domain);

    print.print(new_schedule);
    std::cout << std::endl;

    isl::ast_builder builder(params);
    auto ast = builder.build_ast_from_schedule(new_schedule);

    print.print(ast);

    return schedule;
}

expression generate_code_from_scop(const isl::union_map& schedule)
{
}

expression detect_and_optimize_scops(expression expr)
{
    if (is_scop(expr))
    {
        expr = deduce_loop_bounds(expr);

        auto stmts = analyze_scop(expr);
        auto optimized_schedule = optimize_scop(stmts);

        // return generate_code_from_scop(optimized_schedule);

        return double_literal_expr(0.0);
    }
    else
    {
        // TODO: recurse into childs
        throw 0;
    }
}
}

expression optimize_loops(const expression& expr)
{
    return detect_and_optimize_scops(expr);
}
}
}