#include <qbb/qubus/IR/unary_operator_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qubus
{

unary_operator_expr::unary_operator_expr() : tag_(unary_op_tag::nop)
{
}

unary_operator_expr::unary_operator_expr(unary_op_tag tag_, std::unique_ptr<expression> arg_)
: tag_{tag_}, arg_{take_over_child(arg_)}
{
}

unary_op_tag unary_operator_expr::tag() const
{
    return tag_;
}

const expression& unary_operator_expr::arg() const
{
    return *arg_;
}

unary_operator_expr* unary_operator_expr::clone() const
{
    return new unary_operator_expr(tag_, qubus::clone(*arg_));
}

const expression& unary_operator_expr::child(std::size_t index) const
{
    if (index == 0)
    {
        return *arg_;
    }
    else
    {
        throw 0;
    }
}

std::size_t unary_operator_expr::arity() const
{
    return 1;
}

std::unique_ptr<expression> unary_operator_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 1)
        throw 0;

    return std::make_unique<unary_operator_expr>(tag_, std::move(new_children[0]));
}

bool operator==(const unary_operator_expr& lhs, const unary_operator_expr& rhs)
{
    return lhs.tag() == rhs.tag() && lhs.arg() == rhs.arg();
}

bool operator!=(const unary_operator_expr& lhs, const unary_operator_expr& rhs)
{
    return !(lhs == rhs);
}

}