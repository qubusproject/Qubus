#include <qubus/alias_analysis.hpp>

#include <qubus/affine_constraints.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <boost/range/adaptor/sliced.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/join.hpp>

#include <qubus/util/unused.hpp>

// Basic aliasing rules:
// 0. Direct accesses of the same variable always alias (A == B -> must_alias)
// 1. Accesses of different variables can never alias regardless of qualifiers (A.* != B.* -> noalias)
// 2. Accesses of different sub-objects can never alias
//    2.1 Accesses through different members can never alias (A.foo + A.bar -> noalias)
//    2.2 Accesses of disjoint array regions for the same array-like (sub)-object never alias
// 3. Accesses of the same object through the same members always alias.

namespace qubus
{

namespace
{

alias_result evaluate_alias_query_stage(
    alias_result current_state, const access_qualifier_expr& first_qualifier,
    const access_qualifier_expr& second_qualifier,
    const std::vector<std::function<alias_result(alias_result, const access_qualifier_expr&,
                                                 const access_qualifier_expr&)>>& aliasing_rules)
{
    for (const auto& rule : aliasing_rules)
    {
        auto result = rule(current_state, first_qualifier, second_qualifier);

        if (result != alias_result::may_alias)
            return result;
    }

    return alias_result::may_alias;
}
}

alias_result alias_query_driver::alias(const access& first_access,
                                       const access& second_access) const
{
    if (first_access.variable() != second_access.variable())
        return alias_result::noalias;

    // Assumption: A == B
    alias_result current_state = alias_result::must_alias;

    const auto& first_access_qualifiers = first_access.qualifiers();
    const auto& second_access_qualifiers = second_access.qualifiers();

    for (auto first_access_qualifier = first_access_qualifiers.begin(),
              second_access_qualifier = second_access_qualifiers.begin();
         first_access_qualifier != first_access_qualifiers.end() &&
         second_access_qualifier != second_access_qualifiers.end();
         ++first_access_qualifier, ++second_access_qualifier)
    {
        current_state = evaluate_alias_query_stage(current_state, *first_access_qualifier,
                                                   *second_access_qualifier, aliasing_rules_);

        // If the accesses do not reference the same object, their sub-objects can not either.
        // Therefore, we can just terminate our examination.
        if (current_state == alias_result::noalias)
            return alias_result::noalias;
    }

    return current_state;
}

void alias_query_driver::add_rule(
    std::function<alias_result(alias_result, const access_qualifier_expr&,
                               const access_qualifier_expr&)>
        differentiator)
{
    aliasing_rules_.push_back(std::move(differentiator));
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

alias_result basic_aliasing_rule(alias_result current_state,
                                 const access_qualifier_expr& first_access_qualifier,
                                 const access_qualifier_expr& second_access_qualifier)
{
    using pattern::_;

    pattern::variable<std::string> member1, member2;

    auto m = pattern::make_matcher<
                 std::tuple<const access_qualifier_expr&, const access_qualifier_expr&>, void>()
                 .case_(tuple(member_access(_, member1), member_access(_, member2)),
                        [&] {
                            if (member1.get() != member2.get())
                            {
                                current_state = alias_result::noalias;
                            }
                        })
                 .case_(tuple(member_access(_, _), subscription(_, _)) ||
                            tuple(subscription(_, _), member_access(_, _)),
                        [&] { current_state = alias_result::noalias; })
                 .case_(_, [&] { current_state = alias_result::may_alias; });

    pattern::match(std::tie(first_access_qualifier, second_access_qualifier), m);

    return current_state;
}

boost::optional<affine_constraint>
try_generate_slice_constraints_1D(const affine_expr& var, const expression& offset,
                             const expression& extend, const expression& stride,
                             affine_expr_context& aff_ctx, isl::context& isl_ctx)
{
    auto zero = aff_ctx.create_literal(0);

    auto aff_offset = try_construct_affine_expr(offset, aff_ctx);

    if (!aff_offset)
        return boost::none;

    auto aff_extend = try_construct_affine_expr(extend, aff_ctx);

    if (!aff_extend)
        return boost::none;

    auto aff_stride = try_construct_affine_expr(stride, aff_ctx);

    if (!aff_stride)
        return boost::none;

    auto constraint1 = less_equal(*aff_offset, var);
    auto constraint2 = less(var, *aff_offset + *aff_extend);
    auto constraint3 = equal_to((var - *aff_offset) % *aff_stride, zero);

    affine_constraint constraint = constraint1 && constraint2 && constraint3;

    return constraint;
}

boost::optional<isl::set>
try_generate_slice_region(const std::vector<affine_expr>& vars,
                          const std::vector<std::reference_wrapper<const expression>>& offset,
                          const std::vector<std::reference_wrapper<const expression>>& shape,
                          const std::vector<std::reference_wrapper<const expression>>& strides,
                          affine_expr_context& aff_ctx, isl::context& isl_ctx)
{
    const auto rank = shape.size();

    QBB_ASSERT(rank == vars.size(), "The number of variables need to match the rank.");
    QBB_ASSERT(rank == offset.size(), "The number of offsets need to match the rank.");
    QBB_ASSERT(rank == shape.size(), "The number of extends need to match the rank.");
    QBB_ASSERT(rank == strides.size(), "The number of strides need to match the rank.");

    if (rank == 0)
        return boost::none;

    auto partial_constraint = try_generate_slice_constraints_1D(vars[0], offset[0], shape[0], strides[0],
                                                                aff_ctx, isl_ctx);

    if (!partial_constraint)
        return boost::none;

    affine_constraint constraint = *partial_constraint;

    for (std::size_t i = 1; i < rank; ++i)
    {
        auto partial_constraint = try_generate_slice_constraints_1D(vars[i], offset[i], shape[i], strides[i],
                                                        aff_ctx, isl_ctx);

        if (!partial_constraint)
            return boost::none;

        constraint = std::move(constraint) && *partial_constraint;
    }

    return constraint.convert(isl_ctx);
}

class array_aliasing_rule
{
public:
    array_aliasing_rule(const expression& context_,
                        const value_set_analysis_result& value_set_analysis_,
                        const task_invariants_analysis_result& task_invariants_analysis_,
                        isl::context& isl_ctx_)
    : context_(&context_),
      value_set_analysis_(&value_set_analysis_),
      task_invariants_analysis_(&task_invariants_analysis_),
      isl_ctx_(&isl_ctx_)
    {
    }

    alias_result operator()(alias_result current_state,
                            const access_qualifier_expr& first_access_qualifier,
                            const access_qualifier_expr& second_access_qualifier)
    {
        using pattern::_;

        pattern::variable<std::vector<std::reference_wrapper<const expression>>> indices1, indices2,
            offset1, offset2, shape1, shape2, strides1, strides2;

        auto m = pattern::make_matcher<
                     std::tuple<const access_qualifier_expr&, const access_qualifier_expr&>,
                     alias_result>()
                     .case_(tuple(subscription(_, indices1), subscription(_, indices2)),
                            [&] {

                                if (indices1.get().size() != indices2.get().size())
                                {
                                    return alias_result::noalias;
                                }

                                auto order = indices1.get().size();

                                std::vector<std::reference_wrapper<const expression>> expressions;
                                expressions.reserve(2 * order);

                                expressions.insert(expressions.end(), indices1.get().begin(),
                                                   indices1.get().end());
                                expressions.insert(expressions.end(), indices2.get().begin(),
                                                   indices2.get().end());

                                using boost::adaptors::transformed;

                                auto value_sets = value_set_analysis_->determine_value_sets(
                                    expressions | transformed(unwrap_reference()), *context_);

                                using boost::adaptors::sliced;

                                for (const auto& pair :
                                     boost::range::combine(value_sets | sliced(0, order),
                                                           value_sets | sliced(order, 2 * order)))
                                {
                                    auto common_values = intersect(boost::get<0>(pair).values(),
                                                                   boost::get<1>(pair).values());

                                    if (is_empty(common_values))
                                    {
                                        return alias_result::noalias;
                                    }
                                }

                                return alias_result::may_alias;
                            })
                     .case_(tuple(array_slice(_, offset1, shape1, strides1),
                                  array_slice(_, offset2, shape2, strides2)),
                            [&] {

                                if (shape1.get().size() != shape2.get().size())
                                {
                                    return alias_result::noalias;
                                }

                                affine_expr_context aff_ctx([this](const expression& expr) {
                                    return task_invariants_analysis_->is_invariant(expr, *context_);
                                });

                                auto rank = shape1.get().size();

                                std::vector<affine_expr> vars;
                                vars.reserve(rank);

                                for (std::size_t i = 0; i < rank; ++i)
                                {
                                    variable_declaration tmp(types::integer{});
                                    auto var = aff_ctx.declare_variable(tmp);

                                    vars.push_back(std::move(var));
                                }

                                auto set1 =
                                    try_generate_slice_region(vars, offset1.get(), shape1.get(),
                                                              strides1.get(), aff_ctx, *isl_ctx_);
                                auto set2 =
                                    try_generate_slice_region(vars, offset2.get(), shape2.get(),
                                                              strides2.get(), aff_ctx, *isl_ctx_);

                                if (!set1 || !set2)
                                    return alias_result::may_alias;

                                auto common_values = intersect(*set1, *set2);

                                if (is_empty(common_values))
                                {
                                    return alias_result::noalias;
                                }

                                return alias_result::may_alias;
                            });

        return pattern::match(std::tie(first_access_qualifier, second_access_qualifier), m);
    }

private:
    const expression* context_;
    const value_set_analysis_result* value_set_analysis_;
    const task_invariants_analysis_result* task_invariants_analysis_;
    isl::context* isl_ctx_;
};
}

basic_alias_analysis_result::basic_alias_analysis_result()
{
    driver_.add_rule(basic_aliasing_rule);
}

alias_result basic_alias_analysis_result::alias(const access& first_access,
                                                const access& second_access,
                                                bool enable_caching) const
{
    if (enable_caching)
    {
        auto search_result =
            alias_cache_.find({&first_access.location(), &second_access.location()});

        if (search_result != alias_cache_.end())
            return search_result->second;
    }

    auto result = driver_.alias(first_access, second_access);

    if (enable_caching)
    {
        alias_cache_.emplace(
            std::array<const expression*, 2>{&first_access.location(), &second_access.location()},
            result);
    }

    return result;
}

basic_alias_analysis_result
basic_alias_analysis_pass::run(const expression& QBB_UNUSED(root),
                               analysis_manager& QBB_UNUSED(manager),
                               pass_resource_manager& QBB_UNUSED(resource_manager)) const
{
    return basic_alias_analysis_result();
}

std::vector<analysis_id> basic_alias_analysis_pass::required_analyses() const
{
    return {};
}

alias_analysis_result::alias_analysis_result(
    const expression& root_, const value_set_analysis_result& value_set_analysis_,
    const task_invariants_analysis_result& task_invariants_analysis_, isl::context& isl_ctx_)
{
    driver_.add_rule(basic_aliasing_rule);
    driver_.add_rule(
        array_aliasing_rule(root_, value_set_analysis_, task_invariants_analysis_, isl_ctx_));
}

alias_result alias_analysis_result::alias(const access& first_access, const access& second_access,
                                          bool enable_caching) const
{
    if (enable_caching)
    {
        auto search_result =
            alias_cache_.find({&first_access.location(), &second_access.location()});

        if (search_result != alias_cache_.end())
            return search_result->second;
    }

    auto result = driver_.alias(first_access, second_access);

    if (enable_caching)
    {
        alias_cache_.emplace(
            std::array<const expression*, 2>{&first_access.location(), &second_access.location()},
            result);
    }

    return result;
}

alias_analysis_result alias_analysis_pass::run(const expression& root, analysis_manager& manager,
                                               pass_resource_manager& resource_manager) const
{
    return alias_analysis_result(root, manager.get_analysis<value_set_analysis_pass>(root),
                                 manager.get_analysis<task_invariants_analysis_pass>(root),
                                 resource_manager.get_isl_ctx());
}

std::vector<analysis_id> alias_analysis_pass::required_analyses() const
{
    return {get_analysis_id<value_set_analysis_pass>(),
            get_analysis_id<task_invariants_analysis_pass>()};
}

QUBUS_REGISTER_ANALYSIS_PASS(basic_alias_analysis_pass);
QUBUS_REGISTER_ANALYSIS_PASS(alias_analysis_pass);
}