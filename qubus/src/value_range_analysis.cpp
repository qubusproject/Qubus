#include <qbb/qubus/value_range_analysis.hpp>

#include <qbb/qubus/affine_constraints.hpp>

#include <qbb/qubus/isl/ast_builder.hpp>
#include <qbb/qubus/isl/map.hpp>

#include <utility>

inline namespace qbb
{
namespace qubus
{

value_range_analysis_result::value_range_analysis_result(
    const value_set_analysis_result& value_set_analysis_,
    const axiom_analysis_result& axiom_analysis_, pass_resource_manager& resource_manager_)
: value_set_analysis_(&value_set_analysis_), axiom_analysis_(&axiom_analysis_), resource_manager_(&resource_manager_)
{
}

namespace
{
isl::map build_add_one_map(isl::space s)
{
    auto add_one = isl::map::universe(s);

    auto c = isl::constraint::equality(s);
    c.set_coefficient(isl_dim_in, 0, 1);
    c.set_coefficient(isl_dim_out, 0, -1);
    c.set_constant(1);

    add_one.add_constraint(std::move(c));

    return add_one;
}
}

boost::optional<value_range>
value_range_analysis_result::determine_value_range(const expression& expr,
                                                   const expression& context) const
{
    auto value_set = value_set_analysis_->determine_value_set(expr, context);

    const auto& values = value_set.values();

    if (!values.bounded())
        return boost::none;

    auto value_set_map_space =
        isl::space::from_domain_and_range(values.get_space(), values.get_space());
    auto add_one = build_add_one_map(std::move(value_set_map_space));

    auto lower_bound = lexmin(values);
    auto upper_bound = apply(std::move(add_one), lexmax(values));

    auto ctx = isl::set::universe(isl::space(resource_manager_->get_isl_ctx(), 0));

    auto& aff_ctx = *value_set.ctx();

    for (const auto& axiom : axiom_analysis_->get_valid_axioms(expr))
    {
        auto constraint = try_extract_affine_constraint(axiom.as_expr(), aff_ctx);

        if (constraint)
        {
            auto params = get_params(constraint->convert(resource_manager_->get_isl_ctx()));

            ctx = intersect(std::move(ctx), std::move(params));
        }
    }

    isl::ast_builder ast_builder(ctx);

    auto unit_set = isl::set::universe(isl::space(resource_manager_->get_isl_ctx(), 0, 0));

    // Wrap the bounds in a singular schedule executing them since there are currently
    // no other ways of generating code for them.
    auto lower_bound_schedule = make_map_from_domain_and_range(lower_bound, unit_set);
    auto lower_bound_ast = ast_builder.build_ast_from_schedule(std::move(lower_bound_schedule));

    auto upper_bound_schedule = make_map_from_domain_and_range(upper_bound, unit_set);
    auto upper_bound_ast = ast_builder.build_ast_from_schedule(std::move(upper_bound_schedule));

    // Make sure that we haven't got unexpected code like guard statements.
    // Later on, we might want to handle these  but for now lets just forget about those.
    if (lower_bound_ast.type() != isl_ast_node_user || upper_bound_ast.type() != isl_ast_node_user)
        return boost::none;

    // Extract the actual code from our mock AST.
    // It is also wrapped in a call of an unnamed function.
    auto lower_bound_expr = lower_bound_ast.user_get_expr().get_arg(1);
    auto upper_bound_expr = upper_bound_ast.user_get_expr().get_arg(1);

    auto lower_bound_code = convert_isl_ast_expr_to_qir(lower_bound_expr, aff_ctx);
    auto upper_bound_code = convert_isl_ast_expr_to_qir(upper_bound_expr, aff_ctx);

    return value_range{std::move(lower_bound_code), std::move(upper_bound_code)};
}

value_range_analysis_result
value_range_analysis_pass::run(const expression& root, analysis_manager& manager,
                               pass_resource_manager& resource_manager) const
{
    return value_range_analysis_result(manager.get_analysis<value_set_analysis_pass>(root),
                                       manager.get_analysis<axiom_analysis_pass>(root),
                                       resource_manager);
}

std::vector<analysis_id> value_range_analysis_pass::required_analyses() const
{
    return {get_analysis_id<value_set_analysis_pass>()};
}

QUBUS_REGISTER_ANALYSIS_PASS(value_range_analysis_pass);
}
}
