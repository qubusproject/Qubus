#include <qbb/qubus/IR/type_conversion_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

type_conversion_expr::type_conversion_expr(class type target_type_, std::unique_ptr<expression> arg_)
: target_type_{std::move(target_type_)}, arg_{take_over_child(arg_)}
{
}

class type type_conversion_expr::target_type() const
{
    return target_type_;
}

const expression& type_conversion_expr::arg() const
{
    return *arg_;
}

type_conversion_expr* type_conversion_expr::clone() const
{
    return new type_conversion_expr(target_type_, qbb::qubus::clone(*arg_));
}

const expression& type_conversion_expr::child(std::size_t index) const
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

std::size_t type_conversion_expr::arity() const
{
    return 1;
}

std::unique_ptr<expression> type_conversion_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 1)
        throw 0;

    return std::make_unique<type_conversion_expr>(target_type_, std::move(new_children[0]));
}

bool operator==(const type_conversion_expr& lhs, const type_conversion_expr& rhs)
{
    return lhs.target_type() == rhs.target_type() && lhs.arg() == rhs.arg();
}

bool operator!=(const type_conversion_expr& lhs, const type_conversion_expr& rhs)
{
    return !(lhs == rhs);
}

}
}