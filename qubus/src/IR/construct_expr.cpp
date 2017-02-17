#include <qubus/IR/construct_expr.hpp>

#include <utility>

namespace qubus
{

construct_expr::construct_expr(type result_type_, std::vector<std::unique_ptr<expression>> parameters_)
        : result_type_{std::move(result_type_)}, parameters_(take_over_children(parameters_))
{
}

const type& construct_expr::result_type() const
{
    return result_type_;
}

construct_expr* construct_expr::clone() const
{
    return new construct_expr(result_type_, qubus::clone(parameters_));
}

const expression& construct_expr::child(std::size_t index) const
{
    if (index < parameters_.size())
    {
        return *parameters_[0];
    }
    else
    {
        throw 0;
    }
}

std::size_t construct_expr::arity() const
{
    return parameters_.size();
}

std::unique_ptr<expression> construct_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    return std::make_unique<construct_expr>(result_type_, std::move(new_children));
}

bool operator==(const construct_expr& lhs, const construct_expr& rhs)
{
    return lhs.result_type() == rhs.result_type() && lhs.parameters() == rhs.parameters();
}

bool operator!=(const construct_expr& lhs, const construct_expr& rhs)
{
    return !(lhs == rhs);
}

}

