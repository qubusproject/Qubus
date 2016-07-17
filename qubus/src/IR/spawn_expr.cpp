#include <qbb/qubus/IR/spawn_expr.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

spawn_expr::spawn_expr(function_declaration spawned_plan_, std::vector<std::unique_ptr<expression>> arguments_)
: spawned_plan_(std::move(spawned_plan_)), arguments_(take_over_children(arguments_))
{
}

const function_declaration& spawn_expr::spawned_plan() const
{
    return spawned_plan_;
}

spawn_expr* spawn_expr::clone() const
{
    return new spawn_expr(spawned_plan_, qbb::qubus::clone(arguments_));
}

const expression& spawn_expr::child(std::size_t index) const
{
    if (index < arguments_.size())
    {
        return *arguments_[index];
    }
    else
    {
        throw 0;
    }
}

std::size_t spawn_expr::arity() const
{
    return arguments_.size();
}

std::unique_ptr<expression> spawn_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    return std::make_unique<spawn_expr>(spawned_plan_, std::move(new_children));
}

bool operator==(const spawn_expr& lhs, const spawn_expr& rhs)
{
    return lhs.spawned_plan() == rhs.spawned_plan() && lhs.arguments() == rhs.arguments();
}

bool operator!=(const spawn_expr& lhs, const spawn_expr& rhs)
{
    return !(lhs == rhs);
}
}
}