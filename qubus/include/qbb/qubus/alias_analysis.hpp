#ifndef QUBUS_ALIAS_ANALYSIS_HPP
#define QUBUS_ALIAS_ANALYSIS_HPP

#include <qbb/qubus/variable_access_analysis.hpp>

#include <qbb/qubus/pass_manager.hpp>

#include <array>
#include <functional>
#include <map>

namespace qbb
{
namespace qubus
{

enum class alias_result
{
    noalias,
    may_alias,
    must_alias
};

alias_result alias_using_basic_rules(const access& first_access, const access& second_access);

class basic_alias_analysis_result
{
public:
    alias_result alias(const access& first_access, const access& second_access) const;

private:
    mutable std::map<std::array<const expression*, 2>, alias_result> alias_cache_;
};

class basic_alias_analysis_pass
{
public:
    using result_type = basic_alias_analysis_result;

    basic_alias_analysis_result run(const expression& root, analysis_manager& manager) const;

    std::vector<analysis_id> required_analyses() const;
};

class alias_analysis_result
{
public:
    alias_analysis_result(const expression& root_, analysis_manager& manager_)
    : root_(root_), manager_(manager_)
    {
    }

    alias_result alias(const access& first_access, const access& second_access) const;

private:
    std::reference_wrapper<const expression> root_;
    std::reference_wrapper<analysis_manager> manager_;
};

class alias_analysis_pass
{
public:
    using result_type = alias_analysis_result;

    alias_analysis_result run(const expression& root, analysis_manager& manager) const;

    std::vector<analysis_id> required_analyses() const;
};
}
}

#endif
