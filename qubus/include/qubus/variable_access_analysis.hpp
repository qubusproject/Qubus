#ifndef QUBUS_VARIABLE_ACCESS_ANALYSIS_HPP
#define QUBUS_VARIABLE_ACCESS_ANALYSIS_HPP

#include <qubus/pass_manager.hpp>

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

    const variable_declaration& variable() const
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

class access_set
{
public:
    access_set(const expression& location_, std::vector<access> local_read_accesses_,
               std::vector<access> local_write_accesses_,
               std::vector<std::unique_ptr<access_set>> subsets_);

    access_set(const access_set&) = delete;
    access_set& operator=(const access_set&) = delete;

    access_set(access_set&&) = delete;
    access_set& operator=(access_set&&) = delete;

    const expression& location() const;

    std::vector<access> get_read_accesses() const;

    std::vector<access> get_write_accesses() const;

    auto subsets() const
    {
        return subsets_ | boost::adaptors::indirected;
    }

private:
    std::reference_wrapper<const expression> location_;

    std::vector<access> local_read_accesses_;
    std::vector<access> local_write_accesses_;

    std::vector<std::unique_ptr<access_set>> subsets_;
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

    const access_set& query_accesses_for_location(const expression& location) const;

private:
    std::unique_ptr<variable_access_index> access_index_;
};

class variable_access_analysis
{
public:
    using result_type = variable_access_analyis_result;

    variable_access_analyis_result run(const expression& root, analysis_manager& manager,
                                       pass_resource_manager& resource_manager) const;

    std::vector<analysis_id> required_analyses() const;
};
}

#endif
