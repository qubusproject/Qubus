#ifndef QUBUS_ALIAS_ANALYSIS_HPP
#define QUBUS_ALIAS_ANALYSIS_HPP

#include <qubus/value_set_analysis.hpp>
#include <qubus/variable_access_analysis.hpp>
#include <qubus/task_invariants_analysis.hpp>

#include <qubus/pass_manager.hpp>

#include <array>
#include <functional>
#include <map>
#include <vector>

namespace qubus
{

enum class alias_result
{
    noalias,
    may_alias,
    must_alias
};

class alias_query_driver
{
public:
    alias_result alias(const access& first_access, const access& second_access) const;

    void add_rule(std::function<alias_result(alias_result, const access_qualifier_expr&,
                                             const access_qualifier_expr&)>
                      differentiator);

private:
    std::vector<std::function<alias_result(alias_result, const access_qualifier_expr&,
                                           const access_qualifier_expr&)>>
        aliasing_rules_;
};

class basic_alias_analysis_result
{
public:
    basic_alias_analysis_result();

    alias_result alias(const access& first_access, const access& second_access,
                       bool enable_caching = true) const;

private:
    alias_query_driver driver_;
    mutable std::map<std::array<const expression*, 2>, alias_result> alias_cache_;
};

class basic_alias_analysis_pass
{
public:
    using result_type = basic_alias_analysis_result;

    basic_alias_analysis_result run(const expression& root, analysis_manager& manager,
                                    pass_resource_manager& resource_manager) const;

    std::vector<analysis_id> required_analyses() const;
};

class alias_analysis_result
{
public:
    alias_analysis_result(const expression& root_,
                          const value_set_analysis_result& value_set_analysis_,
                          const task_invariants_analysis_result& task_invariants_analysis_,
                          isl::context& isl_ctx_);

    alias_result alias(const access& first_access, const access& second_access,
                       bool enable_caching = true) const;

private:
    alias_query_driver driver_;
    mutable std::map<std::array<const expression*, 2>, alias_result> alias_cache_;
};

class alias_analysis_pass
{
public:
    using result_type = alias_analysis_result;

    alias_analysis_result run(const expression& root, analysis_manager& manager,
                              pass_resource_manager& resource_manager) const;

    std::vector<analysis_id> required_analyses() const;
};
}

#endif
