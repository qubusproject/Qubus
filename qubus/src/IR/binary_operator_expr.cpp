#include <qbb/qubus/IR/binary_operator_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

binary_operator_expr::binary_operator_expr()
: tag_(binary_op_tag::nop)
{
}

binary_operator_expr::binary_operator_expr(binary_op_tag tag_, expression left_, expression right_)
: tag_{tag_}, left_{std::move(left_)}, right_{std::move(right_)}
{
}

binary_op_tag binary_operator_expr::tag() const
{
    return tag_;
}

expression binary_operator_expr::left() const
{
    return left_;
}
expression binary_operator_expr::right() const
{
    return right_;
}

std::vector<expression> binary_operator_expr::sub_expressions() const
{
    return {left_, right_};
}

expression
binary_operator_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 2, "invalid number of subexpressions");

    return binary_operator_expr(tag_, subexprs[0], subexprs[1]);
}

annotation_map& binary_operator_expr::annotations() const
{
    return annotations_;
}

annotation_map& binary_operator_expr::annotations()
{
    return annotations_;
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