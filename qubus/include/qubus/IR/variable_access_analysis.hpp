#ifndef QUBUS_VARIABLE_ACCESS_ANALYSIS_HPP
#define QUBUS_VARIABLE_ACCESS_ANALYSIS_HPP

#include <qubus/IR/pass_manager.hpp>

#include <qubus/IR/access_qualifier.hpp>
#include <qubus/IR/expression.hpp>
#include <qubus/IR/variable_declaration.hpp>
#include <qubus/IR/variable_ref_expr.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/unreachable.hpp>

#include <boost/optional.hpp>
#include <boost/range/adaptor/indirected.hpp>

#include <functional>
#include <memory>
#include <tuple>
#include <vector>

namespace qubus
{
class access
{
public:
    explicit access(const access_expr& access_);

    access(const variable_ref_expr& direct_access_,
           std::vector<std::reference_wrapper<const access_qualifier_expr>> qualifiers_);

    const variable_ref_expr& base() const
    {
        return direct_access_.get();
    }

    std::shared_ptr<const variable_declaration> variable() const
    {
        return base().declaration();
    }

    bool is_qualified() const
    {
        return !qualifiers_.empty();
    }

    decltype(auto) qualifiers() const
    {
        return qualifiers_;
    }

    const expression& location() const;

private:
    explicit access(std::tuple<std::reference_wrapper<const variable_ref_expr>,
                               std::vector<std::reference_wrapper<const access_qualifier_expr>>>
                        data_);

    std::reference_wrapper<const variable_ref_expr> direct_access_;
    std::vector<std::reference_wrapper<const access_qualifier_expr>> qualifiers_;

    std::reference_wrapper<const expression> location_;
};

class access_set_node;

enum class access_kind
{
    external, // Only accesses external to the queried expression.
    all // All accesses within the queried expression.
};

class access_set
{
public:
    explicit access_set(const access_set_node& root_, access_kind kind_);

    const expression& location() const;

    std::vector<access> get_read_accesses() const;
    std::vector<access> get_write_accesses() const;
private:
    const access_set_node* root_;
    access_kind kind_;
};

class variable_access_index;

class variable_access_analyis_result
{
public:
    explicit variable_access_analyis_result(std::unique_ptr<variable_access_index> access_index_);

    ~variable_access_analyis_result();

    variable_access_analyis_result(const variable_access_analyis_result&) = delete;
    variable_access_analyis_result& operator=(const variable_access_analyis_result&) = delete;

    variable_access_analyis_result(variable_access_analyis_result&&);
    variable_access_analyis_result& operator=(variable_access_analyis_result&&);

    access_set query_accesses_for_location(const expression& location, access_kind kind = access_kind::external) const;

private:
    std::unique_ptr<variable_access_index> access_index_;
};

class variable_access_analysis
{
public:
    using result_type = variable_access_analyis_result;

    variable_access_analyis_result run(const expression& root, expression_analysis_manager& manager,
                                       pass_resource_manager& resource_manager) const;

    std::vector<analysis_id> required_analyses() const;
};
}

#endif
