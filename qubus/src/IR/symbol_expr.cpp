#include <qubus/IR/symbol_expr.hpp>

#include <utility>

namespace qubus
{

symbol_expr::symbol_expr(symbol_id id) noexcept
: m_id(std::move(id))
{
}

const symbol_id& symbol_expr::id() const noexcept
{
    return m_id;
}

symbol_expr* symbol_expr::clone() const
{
    return new symbol_expr(id());
}

const expression& symbol_expr::child(std::size_t index) const
{
    throw 0;
}

std::size_t symbol_expr::arity() const
{
    return 0;
}

std::unique_ptr<expression> symbol_expr::substitute_subexpressions(
    std::vector<std::unique_ptr<expression>> new_children) const
{
    if (!new_children.empty())
        throw 0;

    return std::make_unique<symbol_expr>(id());
}

bool operator==(const symbol_expr& lhs, const symbol_expr& rhs)
{
    return lhs.id() == rhs.id();
}

bool operator!=(const symbol_expr& lhs, const symbol_expr& rhs)
{
    return !(lhs == rhs);
}

}