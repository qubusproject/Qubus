#include <qbb/qubus/alias_analysis.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <qbb/util/unused.hpp>

namespace qbb
{
namespace qubus
{

alias_result alias_using_basic_rules(const access& first_access, const access& second_access)
{
    // Basic aliasing rules:
    // 0. Direct accesses of the same variable always alias (A == B -> must_alias)
    // 1. Accesses of different variables can never alias regardless of qualifiers (A.* != B.* -> noalias)
    // 2. Accesses of different sub-objects can never alias
    //    2.1 Accesses through different members can never alias (A.foo + A.bar -> noalias)
    //    2.2 Accesses of disjoint array regions for the same array-like (sub)-object never alias
    // 3. Accesses of the same object through the same members always alias.

    using pattern::_;

    if (first_access.variable() != second_access.variable())
        return alias_result::noalias;

    // Assumption 1: A == B
    alias_result current_state = alias_result::must_alias;

    const auto& first_access_qualifiers = first_access.qualifiers();
    const auto& second_access_qualifiers = second_access.qualifiers();

    pattern::variable<std::string> member1, member2;

    auto m =
        pattern::make_matcher<
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

    for (auto first_access_qualifier = first_access_qualifiers.begin(),
              second_access_qualifier = second_access_qualifiers.begin();
         first_access_qualifier != first_access_qualifiers.end() &&
         second_access_qualifier != second_access_qualifiers.end();
         ++first_access_qualifier, ++second_access_qualifier)
    {
        pattern::match(std::tie(*first_access_qualifier, *second_access_qualifier), m);

        if (current_state == alias_result::noalias)
            return alias_result::noalias;
    }

    return current_state;
}

alias_result basic_alias_analysis_result::alias(const access& first_access,
                                                const access& second_access) const
{
    auto search_result = alias_cache_.find({&first_access.location(), &second_access.location()});

    if (search_result != alias_cache_.end())
    {
        return search_result->second;
    }
    else
    {
        auto result = alias_using_basic_rules(first_access, second_access);

        alias_cache_.emplace(
            std::array<const expression*, 2>{&first_access.location(), &second_access.location()},
            result);

        return result;
    }
}

basic_alias_analysis_result
basic_alias_analysis_pass::run(const expression& QBB_UNUSED(root),
                               analysis_manager& QBB_UNUSED(manager)) const
{
    return basic_alias_analysis_result();
}

std::vector<analysis_id> basic_alias_analysis_pass::required_analyses() const
{
    return {};
}

alias_result alias_analysis_result::alias(const access& first_access, const access& second_access) const
{
    auto& basic_alias_analysis = manager_.get().get_analysis<basic_alias_analysis_pass>(root_);

    auto basic_result = basic_alias_analysis.alias(first_access, second_access);

    return basic_result;
}

alias_analysis_result alias_analysis_pass::run(const expression& root, analysis_manager& manager) const
{
    return alias_analysis_result(root, manager);
}

std::vector<analysis_id> alias_analysis_pass::required_analyses() const
{
    return {get_analysis_id<basic_alias_analysis_pass>()};
}

QUBUS_REGISTER_ANALYSIS_PASS(basic_alias_analysis_pass);
QUBUS_REGISTER_ANALYSIS_PASS(alias_analysis_pass);

}
}