#include <qbb/qubus/value_set_analysis.hpp>

#include <qbb/qubus/isl/map.hpp>

#include <boost/range/adaptor/transformed.hpp>

#include <qbb/util/assert.hpp>

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

std::vector<value_set> value_set_analysis_result::determine_value_sets(
    boost::any_range<const expression&, boost::forward_traversal_tag> expressions,
    const expression& context) const
{
    QBB_ASSERT(axiom_analysis_ != nullptr, "Invalid value_set_analysis_result object.");

    auto ctx = std::make_shared<affine_expr_context>([this, &context](const expression& expr) {
        return task_invariants_analysis_->is_invariant(expr, context);
    });

    std::vector<value_set> value_sets;

    for (const auto& expr : expressions)
    {
        auto axioms = axiom_analysis_->get_valid_axioms(expr);

        auto analyzed_expr = try_construct_affine_expr(expr, *ctx);

        if (analyzed_expr)
        {
            std::vector<affine_constraint> affine_axioms;

            for (const auto& axiom : axioms)
            {
                auto expr = try_extract_affine_constraint(axiom.as_expr(), *ctx);

                if (expr)
                {
                    affine_axioms.push_back(*std::move(expr));
                }
            }

            auto domain = isl::set::universe(ctx->construct_corresponding_space(*isl_ctx_));

            for (const auto& axiom : affine_axioms)
            {
                auto constraint = axiom.convert(*isl_ctx_);

                domain = intersect(std::move(domain), constraint);
            }

            domain = simplify(std::move(domain));

            auto map = isl::map::from_affine_expr(analyzed_expr->convert(*isl_ctx_));

            auto value_set = apply(map, domain);

            value_sets.emplace_back(std::move(value_set), ctx);
        }
        else
        {
            value_sets.emplace_back(isl::set::universe(isl::space(*isl_ctx_, 0, 1)), ctx);
        }
    }

    return value_sets;
}

namespace
{
struct unwrap_reference
{
    template <typename T>
    T& operator()(const std::reference_wrapper<T>& wrapper) const
    {
        return wrapper.get();
    }
};
}

value_set value_set_analysis_result::determine_value_set(const expression& expr,
                                                         const expression& context) const
{
    std::vector<std::reference_wrapper<const expression>> expressions{expr};

    using boost::adaptors::transformed;

    auto value_sets = determine_value_sets(expressions | transformed(unwrap_reference()), context);

    QBB_ASSERT(value_sets.size() == 1, "Expecting only one value set.");

    return value_sets[0];
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
