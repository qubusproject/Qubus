#include <qbb/qubus/IR/member_access_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

member_access_expr::member_access_expr(std::unique_ptr<access_expr> object_, std::string member_name_)
: object_(take_over_child(object_)), member_name_(std::move(member_name_))
{
}

const access_expr& member_access_expr::object() const
{
    return *object_;
}

const std::string& member_access_expr::member_name() const
{
    return member_name_;
}

const access_expr& member_access_expr::qualified_access() const
{
    return object();
}

member_access_expr* member_access_expr::clone() const
{
    return new member_access_expr(qbb::qubus::clone(*object_), member_name_);
}

const expression& member_access_expr::child(std::size_t index) const
{
    if (index == 0)
    {
        return *object_;
    }
    else
    {
        throw 0;
    }
}

std::size_t member_access_expr::arity() const
{
    return 1;
}

std::unique_ptr<expression> member_access_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    std::unique_ptr<access_expr> qualified_access(dynamic_cast<access_expr*>(new_children[0].release()));

    if (!qualified_access)
        throw 0;

    if (new_children.size() != 1)
        throw 0;

    return std::make_unique<member_access_expr>(std::move(qualified_access), member_name_);
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
