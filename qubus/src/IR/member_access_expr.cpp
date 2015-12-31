#include <qbb/qubus/IR/member_access_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

member_access_expr::member_access_expr(expression object_, std::string member_name_)
: object_(std::move(object_)), member_name_(std::move(member_name_))
{
}

const expression& member_access_expr::object() const
{
    return object_;
}

const std::string& member_access_expr::member_name() const
{
    return member_name_;
}

std::vector<expression> member_access_expr::sub_expressions() const
{
    return { object_ };
}

expression member_access_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 1, "invalid number of subexpressions");

    return member_access_expr(subexprs[0], member_name_);
}

annotation_map& member_access_expr::annotations() const
{
    return annotations_;
}

annotation_map& member_access_expr::annotations()
{
    return annotations_;
}

bool operator==(const member_access_expr& lhs, const member_access_expr& rhs)
{
    return lhs.member_name() == rhs.member_name() && lhs.object() == rhs.object();
}

bool operator!=(const member_access_expr& lhs, const member_access_expr& rhs)
{
    return !(lhs == rhs);
}

}
}
