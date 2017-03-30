#include <qubus/qtl/IR/object_expr.hpp>

#include <utility>

namespace qubus
{
namespace qtl
{

object_expr::object_expr(object obj_) : obj_(std::move(obj_))
{
}

const object& object_expr::obj() const
{
    return obj_;
}

object_expr* object_expr::clone() const
{
    return new object_expr(obj_);
}

const expression& object_expr::child(std::size_t index) const
{
    throw 0;
}

std::size_t object_expr::arity() const
{
    return 0;
}

std::unique_ptr<expression>
object_expr::substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const
{
    if (!new_children.empty())
        throw 0;

    return ::qubus::clone(*this);
}

bool operator==(const object_expr& lhs, const object_expr& rhs)
{
    return lhs.obj() == rhs.obj();
}

bool operator!=(const object_expr& lhs, const object_expr& rhs)
{
    return !(lhs == rhs);
}

}
}
