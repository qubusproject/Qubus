#ifndef QUBUS_AFFINE_CONSTRAINTS_HPP
#define QUBUS_AFFINE_CONSTRAINTS_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>

#include <qbb/qubus/isl/affine_expr.hpp>
#include <qbb/qubus/isl/context.hpp>
#include <qbb/qubus/isl/set.hpp>
#include <qbb/qubus/isl/space.hpp>

#include <boost/optional.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace qbb
{
namespace qubus
{

class affine_expr_context;

class affine_constraint;

class affine_expr
{
public:
    affine_expr() = default;

    affine_expr(const affine_expr& other);
    affine_expr(affine_expr&& other) noexcept;

    affine_expr& operator=(const affine_expr& other);
    affine_expr& operator=(affine_expr&& other) noexcept;

    const expression& underlying_expression() const;

    isl::affine_expr convert(isl::context& isl_ctx) const;

    friend affine_expr operator+(affine_expr lhs, affine_expr rhs);
    friend affine_expr operator-(affine_expr lhs, affine_expr rhs);
    friend affine_expr operator*(affine_expr lhs, affine_expr rhs);
    friend affine_expr operator/(affine_expr lhs, affine_expr rhs);
    friend affine_expr operator-(affine_expr arg);

    friend bool is_const(const affine_expr& expr);

    friend affine_constraint equal_to(affine_expr lhs, affine_expr rhs);
    friend affine_constraint not_equal_to(affine_expr lhs, affine_expr rhs);
    friend affine_constraint less(affine_expr lhs, affine_expr rhs);
    friend affine_constraint greater(affine_expr lhs, affine_expr rhs);
    friend affine_constraint less_equal(affine_expr lhs, affine_expr rhs);
    friend affine_constraint greater_equal(affine_expr lhs, affine_expr rhs);
private:
    friend class affine_expr_context;

    affine_expr(std::unique_ptr<expression> expr_, affine_expr_context& ctx_);

    std::unique_ptr<expression> expr_;
    affine_expr_context* ctx_;
};

class affine_expr_context
{
public:
    explicit affine_expr_context(std::function<bool(const expression&)> invariant_checker_);

    affine_expr declare_variable(const variable_declaration& var);

    affine_expr define_constant(const expression& value);
    affine_expr create_literal(util::index_t value);
    std::unique_ptr<expression> get_constant(const std::string& name) const;

    isl::space construct_corresponding_space(isl::context& isl_ctx) const;

    const std::function<bool(const expression&)>& get_invariant_checker() const;
private:
    std::function<bool(const expression&)> invariant_checker_;

    std::size_t free_variable_id_ = 0;
    std::map<variable_declaration, std::string> variable_table_;

    std::size_t free_constant_id_ = 0;
    std::vector<std::tuple<const expression*, std::string>> constant_table_;
};

class affine_constraint
{
public:
    affine_constraint(std::unique_ptr<expression> expr_, affine_expr_context& ctx_);

    isl::set convert(isl::context& isl_ctx) const;

    friend affine_constraint operator&&(affine_constraint lhs, affine_constraint rhs);
    friend affine_constraint operator||(affine_constraint lhs, affine_constraint rhs);
    friend affine_constraint operator!(affine_constraint arg);
private:
    std::unique_ptr<expression> expr_;
    affine_expr_context* ctx_;
};

boost::optional<affine_expr> try_construct_affine_expr(const expression& expr,
                                                       affine_expr_context& ctx);

boost::optional<affine_constraint> try_extract_affine_constraint(const expression& expr,
                                                                 affine_expr_context& ctx);
}
}

#endif
