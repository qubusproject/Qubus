#include <qbb/qubus/IR/binary_operator_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

inline namespace qbb
{
namespace qubus
{

binary_operator_expr::binary_operator_expr() : tag_(binary_op_tag::nop)
{
}

binary_operator_expr::binary_operator_expr(binary_op_tag tag_, std::unique_ptr<expression> left_,
                                           std::unique_ptr<expression> right_)
: tag_{tag_}, left_{take_over_child(left_)}, right_{take_over_child(right_)}
{
}

binary_op_tag binary_operator_expr::tag() const
{
    return tag_;
}

const expression& binary_operator_expr::left() const
{
    return *left_;
}

const expression& binary_operator_expr::right() const
{
    return *right_;
}

binary_operator_expr* binary_operator_expr::clone() const
{
    return new binary_operator_expr(tag_, qbb::qubus::clone(*left_), qbb::qubus::clone(*right_));
}

const expression& binary_operator_expr::child(std::size_t index) const
{
    if (index == 0)
    {
        return *left_;
    }
    else if (index == 1)
    {
        return *right_;
    }
    else
    {
        throw 0;
    }
}

std::size_t binary_operator_expr::arity() const
{
    return 2;
}

std::unique_ptr<expression> binary_operator_expr::substitute_subexpressions(
    std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 2)
        throw 0;

    return std::make_unique<binary_operator_expr>(tag_, std::move(new_children[0]), std::move(new_children[1]));
}

bool operator==(const binary_operator_expr& lhs, const binary_operator_expr& rhs)
{
    return lhs.tag() == rhs.tag() && lhs.left() == rhs.left() && lhs.right() == rhs.right();
}

bool operator!=(const binary_operator_expr& lhs, const binary_operator_expr& rhs)
{
    return !(lhs == rhs);
}
}
}