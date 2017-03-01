#include <qubus/IR/foreign_call_expr.hpp>

#include <qubus/util/unused.hpp>

#include <utility>

namespace qubus
{

foreign_call_expr::foreign_call_expr(foreign_computelet computelet_,
                                     std::vector<std::unique_ptr<expression>> args_,
                                     std::unique_ptr<expression> result_)
: computelet_(std::move(computelet_)),
  args_(take_over_children(args_)),
  result_(take_over_child(result_))
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

const expression& foreign_call_expr::result() const
{
    return *result_;
}

foreign_call_expr* foreign_call_expr::clone() const
{
    return new foreign_call_expr(computelet_, qubus::clone(args_), qubus::clone(*result_));
}

const expression& foreign_call_expr::child(std::size_t index) const
{
    if (index < args_.size())
    {
        return *args_[index];
    }
    else if (index == args_.size())
    {
        return *result_;
    }
    else
    {
        throw 0;
    }
}

std::size_t foreign_call_expr::arity() const
{
    return args_.size() + 1;
}

std::unique_ptr<expression> foreign_call_expr::substitute_subexpressions(
    std::vector<std::unique_ptr<expression>> new_children) const
{
    std::vector<std::unique_ptr<expression>> arguments;

    for (std::size_t i = 0; i < args_.size(); ++i)
    {
        arguments.push_back(std::move(new_children[i]));
    }

    return std::make_unique<foreign_call_expr>(computelet_, std::move(arguments),
                                               std::move(new_children[args_.size()]));
}

bool operator==(const foreign_call_expr& QUBUS_UNUSED(lhs),
                const foreign_call_expr& QUBUS_UNUSED(rhs))
{
    return false;
}

bool operator!=(const foreign_call_expr& lhs, const foreign_call_expr& rhs)
{
    return !(lhs == rhs);
}
}
