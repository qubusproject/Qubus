#include <qbb/qubus/value_set_analysis.hpp>

#include <qbb/qubus/isl/map.hpp>

#include <qbb/util/assert.hpp>

#include <qbb/qubus/affine_constraints.hpp>

namespace qbb
{
namespace qubus
{

value_set_analysis_result::value_set_analysis_result(
    const axiom_analysis_result& axiom_analysis_,
    const task_invariants_analysis_result& task_invariants_analysis_, isl::context& isl_ctx_)
: axiom_analysis_(&axiom_analysis_),
  task_invariants_analysis_(&task_invariants_analysis_),
  isl_ctx_(&isl_ctx_)
{
}

isl::set value_set_analysis_result::determine_value_set(const expression& expr,
                                                        const expression& context) const
{
    QBB_ASSERT(axiom_analysis_ != nullptr, "Invalid value_set_analysis_result object.");

    auto axioms = axiom_analysis_->get_valid_axioms(expr);

    affine_expr_context ctx([this, &context](const expression& expr) {
        return task_invariants_analysis_->is_invariant(expr, context);
    });

    auto analyzed_expr = try_construct_affine_expr(expr, ctx);

    if (analyzed_expr)
    {
        std::vector<affine_constraint> affine_axioms;

        for (const auto& axiom : axioms)
        {
            auto expr = try_extract_affine_constraint(axiom.as_expr(), ctx);

            if (expr)
            {
                affine_axioms.push_back(*std::move(expr));
            }
        }

        auto domain = isl::set::universe(ctx.construct_corresponding_space(*isl_ctx_));

        for (const auto& axiom : affine_axioms)
        {
            auto constraint = axiom.convert(*isl_ctx_);

            domain = intersect(std::move(domain), constraint);
        }

        domain = simplify(std::move(domain));

        auto map = isl::map::from_affine_expr(analyzed_expr->convert(*isl_ctx_));

        auto value_set = apply(map, domain);

        return value_set;
    }
    else
    {
        return isl::set::universe(isl::space(*isl_ctx_, 0, 1));
    }
}

value_set_analysis_result
value_set_analysis_pass::run(const expression& root, analysis_manager& manager,
                             pass_resource_manager& resource_manager) const
{
    return value_set_analysis_result(manager.get_analysis<axiom_analysis_pass>(root),
                                     manager.get_analysis<task_invariants_analysis_pass>(root),
                                     resource_manager.get_isl_ctx());
}

std::vector<analysis_id> value_set_analysis_pass::required_analyses() const
{
    return {get_analysis_id<axiom_analysis_pass>()};
}

QUBUS_REGISTER_ANALYSIS_PASS(value_set_analysis_pass);
}
}
