#include <qubus/IR/foreign_call_expr.hpp>

#include <qubus/util/unused.hpp>

#include <utility>

namespace qubus
{

foreign_call_expr::foreign_call_expr(foreign_computelet computelet_, std::vector<std::unique_ptr<expression>> args_)
: computelet_(std::move(computelet_)), args_(take_over_children(args_))
{
}

type foreign_call_expr::result_type() const
{
    return computelet_.result_type();
}

const foreign_computelet& foreign_call_expr::computelet() const
{
    return computelet_;
}

foreign_call_expr* foreign_call_expr::clone() const
{
    return new foreign_call_expr(computelet_, qubus::clone(args_));
}

const expression& foreign_call_expr::child(std::size_t index) const
{
    if (index < args_.size())
    {
        return *args_[index];
    }
    else
    {
        throw 0;
    }
}

std::size_t foreign_call_expr::arity() const
{
    return args_.size();
}

std::unique_ptr<expression> foreign_call_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    return std::make_unique<foreign_call_expr>(computelet_, std::move(new_children));
}

bool operator==(const foreign_call_expr& QBB_UNUSED(lhs), const foreign_call_expr& QBB_UNUSED(rhs))
{
    return false;
}

bool operator!=(const foreign_call_expr& lhs, const foreign_call_expr& rhs)
{
    return !(lhs == rhs);
}

}
