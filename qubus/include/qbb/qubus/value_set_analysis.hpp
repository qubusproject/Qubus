#ifndef QUBUS_VALUE_SET_ANALYSIS_HPP
#define QUBUS_VALUE_SET_ANALYSIS_HPP

#include <qbb/qubus/axiom_analysis.hpp>
#include <qbb/qubus/task_invariants_analysis.hpp>

#include <qbb/qubus/affine_constraints.hpp>

#include <qbb/qubus/pass_manager.hpp>

#include <qbb/qubus/IR/expression.hpp>

#include <qbb/qubus/isl/set.hpp>

#include <boost/range/any_range.hpp>

#include <functional>
#include <memory>
#include <utility>

namespace qbb
{
namespace qubus
{

class value_set
{
public:
    value_set(isl::set values_, std::shared_ptr<affine_expr_context> ctx_)
    : values_(std::move(values_)), ctx_(std::move(ctx_))
    {
    }

    const isl::set& values() const
    {
        return values_;
    }

    const std::shared_ptr<affine_expr_context>& ctx() const
    {
        return ctx_;
    }

private:
    isl::set values_;
    std::shared_ptr<affine_expr_context> ctx_;
};

class value_set_analysis_result
{
public:
    explicit value_set_analysis_result(
        const axiom_analysis_result& axiom_analysis_,
        const task_invariants_analysis_result& task_invariants_analysis_, isl::context& isl_ctx_);

    std::vector<value_set>
    determine_value_sets(boost::any_range<const expression&, boost::forward_traversal_tag> expressions,
                         const expression& context) const;
    value_set determine_value_set(const expression& expr, const expression& context) const;

private:
    const axiom_analysis_result* axiom_analysis_;
    const task_invariants_analysis_result* task_invariants_analysis_;
    isl::context* isl_ctx_;
};

class value_set_analysis_pass
{
public:
    using result_type = value_set_analysis_result;

    value_set_analysis_result run(const expression& root, analysis_manager& manager,
                                  pass_resource_manager& resource_manager) const;

    std::vector<analysis_id> required_analyses() const;
};
}
}

#endif
