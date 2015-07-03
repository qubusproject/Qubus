#include <qbb/kubus/IR/if_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

if_expr::if_expr(expression condition_, expression then_branch_)
: condition_(std::move(condition_)), then_branch_(std::move(then_branch_))
{

}

if_expr::if_expr(expression condition_, expression then_branch_, expression else_branch_)
: condition_(std::move(condition_)), then_branch_(std::move(then_branch_)),
  else_branch_(std::move(else_branch_))
{
}

expression if_expr::condition() const
{
    return condition_;
}

expression if_expr::then_branch() const
{
    return then_branch_;
}

boost::optional<expression> if_expr::else_branch() const
{
    return else_branch_;
}

std::vector<expression> if_expr::sub_expressions() const
{
    if (else_branch_)
    {
        return std::vector<expression>{condition_, then_branch_, *else_branch_};
    }
    else
    {
        return std::vector<expression>{condition_, then_branch_};
    }
}

expression if_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 2 || subexprs.size() == 3, "invalid number of subexpressions");

    if (subexprs.size() == 2)
    {
        return if_expr(subexprs[0], subexprs[1]);
    }
    else
    {
        return if_expr(subexprs[0], subexprs[1], subexprs[2]);
    }
}

annotation_map& if_expr::annotations() const
{
    return annotations_;
}

annotation_map& if_expr::annotations()
{
    return annotations_;
}

bool operator==(const if_expr& lhs, const if_expr& rhs)
{
    return lhs.condition() == rhs.condition() && lhs.then_branch() == rhs.then_branch() &&
           lhs.else_branch() == rhs.else_branch();
}

bool operator!=(const if_expr& lhs, const if_expr& rhs)
{
    return !(lhs == rhs);
}
}
}
