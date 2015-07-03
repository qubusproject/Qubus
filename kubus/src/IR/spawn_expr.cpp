#include <qbb/kubus/IR/spawn_expr.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

spawn_expr::spawn_expr(function_declaration spawned_plan_, std::vector<expression> arguments_)
: spawned_plan_(std::move(spawned_plan_)), arguments_(std::move(arguments_))
{
}

const function_declaration& spawn_expr::spawned_plan() const
{
    return spawned_plan_;
}

const std::vector<expression>& spawn_expr::arguments() const
{
    return arguments_;
}

std::vector<expression> spawn_expr::sub_expressions() const
{
    return arguments_;
}

expression spawn_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    return spawn_expr(spawned_plan_, subexprs);
}

annotation_map& spawn_expr::annotations() const
{
    return annotations_;
}

annotation_map& spawn_expr::annotations()
{
    return annotations_;
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