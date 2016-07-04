#include <qbb/qubus/IR/unary_operator_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

unary_operator_expr::unary_operator_expr() : tag_(unary_op_tag::nop)
{
}

unary_operator_expr::unary_operator_expr(unary_op_tag tag_, expression arg_)
: tag_{tag_}, arg_{std::move(arg_)}
{
}

unary_op_tag unary_operator_expr::tag() const
{
    return tag_;
}

expression unary_operator_expr::arg() const
{
    return arg_;
}

std::vector<expression> unary_operator_expr::sub_expressions() const
{
    return {arg_};
}

expression unary_operator_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 1, "invalid number of subexpressions");
    
    return unary_operator_expr(tag_, subexprs[0]);
}

annotation_map& unary_operator_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& unary_operator_expr::annotations()
{
    return annotations_;
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
}