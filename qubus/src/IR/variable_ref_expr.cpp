#include <qbb/qubus/IR/variable_ref_expr.hpp>

#include <qbb/util/assert.hpp>
#include <qbb/util/unused.hpp>

namespace qubus
{

variable_ref_expr::variable_ref_expr(variable_declaration declaration_) : declaration_(declaration_)
{
}

const variable_declaration& variable_ref_expr::declaration() const
{
    return declaration_;
}

variable_ref_expr* variable_ref_expr::clone() const
{
    return new variable_ref_expr(declaration_);
}

const expression& variable_ref_expr::child(std::size_t QBB_UNUSED(index)) const
{
    throw 0;
}

std::size_t variable_ref_expr::arity() const
{
    return 0;
}

std::unique_ptr<expression> variable_ref_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 0)
        throw 0;

    return std::make_unique<variable_ref_expr>(declaration_);
}

bool operator==(const variable_ref_expr& lhs, const variable_ref_expr& rhs)
{
    return lhs.declaration() == rhs.declaration();
}

bool operator!=(const variable_ref_expr& lhs, const variable_ref_expr& rhs)
{
    return !(lhs == rhs);
}
}