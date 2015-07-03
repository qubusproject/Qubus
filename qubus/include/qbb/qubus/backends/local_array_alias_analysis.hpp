#ifndef QBB_QUBUS_LOCAL_ALIAS_ANALYSIS_HPP
#define QBB_QUBUS_LOCAL_ALIAS_ANALYSIS_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>

#include <qbb/qubus/backends/reference.hpp>
#include <qbb/qubus/backends/alias_info.hpp>
#include <qbb/qubus/backends/llvm_environment.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/util/make_unique.hpp>

#include <map>
#include <vector>
#include <memory>

namespace qbb
{
namespace qubus
{

class index_expr
{
public:
    explicit index_expr(util::index_t value_);
    explicit index_expr(util::handle var_id);
    index_expr(std::map<util::handle, util::index_t> coefficients_, util::index_t constant_);

    const std::map<util::handle, util::index_t>& coefficients() const;
    util::index_t constant() const;

    bool is_const() const;
    bool is_nonzero_const() const;
    bool is_zero() const;

    void simplify() const;

private:
    mutable std::map<util::handle, util::index_t> coefficients_;
    util::index_t constant_;
};

index_expr operator+(index_expr lhs, index_expr rhs);
index_expr operator-(index_expr lhs, index_expr rhs);
index_expr operator*(index_expr lhs, index_expr rhs);
index_expr operator-(index_expr arg);

bool operator==(const index_expr& lhs, const index_expr& rhs);
bool operator!=(const index_expr& lhs, const index_expr& rhs);
bool operator<(const index_expr& lhs, const index_expr& rhs);

std::vector<index_expr> index_expr_from_expression(const std::vector<expression>& indices);
bool contains_loops(const expression& expr);

class local_array_access_alias_analysis_impl;

class local_array_access_alias_analysis
{
public:
    local_array_access_alias_analysis(util::handle token_, llvm_environment& env_);
    ~local_array_access_alias_analysis();

    local_array_access_alias_analysis(const local_array_access_alias_analysis&) = delete;
    local_array_access_alias_analysis& operator=(const local_array_access_alias_analysis&) = delete;

    alias_info query(variable_declaration accessed_array, std::vector<expression> indices,
                     reference data_ref);

private:
    std::unique_ptr<local_array_access_alias_analysis_impl> impl_;
};
}
}

#endif
