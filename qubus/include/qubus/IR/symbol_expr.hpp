#ifndef QUBUS_IR_SYMBOL_EXPR_HPP
#define QUBUS_IR_SYMBOL_EXPR_HPP

#include <hpx/config.hpp>

#include <qubus/IR/annotations.hpp>
#include <qubus/IR/expression.hpp>
#include <qubus/IR/type.hpp>
#include <qubus/IR/symbol_id.hpp>

#include <qubus/util/integers.hpp>
#include <qubus/util/unused.hpp>

#include <type_traits>
#include <vector>

namespace qubus
{

class symbol_expr final : public expression_base<symbol_expr>
{

public:
    explicit symbol_expr(symbol_id id) noexcept;

    const symbol_id& id() const noexcept;

    symbol_expr* clone() const final;

    const expression& child(std::size_t index) const final;

    std::size_t arity() const final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const final;

private:
    symbol_id m_id;

    mutable annotation_map annotations_;
};

bool operator==(const symbol_expr& lhs, const symbol_expr& rhs);
bool operator!=(const symbol_expr& lhs, const symbol_expr& rhs);

}

#endif
