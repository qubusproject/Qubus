#include <qbb/qubus/IR/intrinsic_function_expr.hpp>

#include <utility>

namespace qubus
{

intrinsic_function_expr::intrinsic_function_expr(std::string name_,
                                                 std::vector<std::unique_ptr<expression>> args_)
: name_{std::move(name_)}, args_(take_over_children(args_))
{
}

const std::string& intrinsic_function_expr::name() const
{
    return name_;
}

intrinsic_function_expr* intrinsic_function_expr::clone() const
{
    return new intrinsic_function_expr(name_, qubus::clone(args_));
}

const expression& intrinsic_function_expr::child(std::size_t index) const
{
    return *args_.at(index);
}

std::size_t intrinsic_function_expr::arity() const
{
    return args_.size();
}

std::unique_ptr<expression> intrinsic_function_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    return std::make_unique<intrinsic_function_expr>(name_, std::move(new_children));
}

bool operator==(const intrinsic_function_expr& lhs, const intrinsic_function_expr& rhs)
{
    return lhs.name() == rhs.name() && lhs.args() == rhs.args();
}

bool operator!=(const intrinsic_function_expr& lhs, const intrinsic_function_expr& rhs)
{
    return !(lhs == rhs);
}
}