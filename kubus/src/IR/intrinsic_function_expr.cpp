#include <qbb/kubus/IR/intrinsic_function_expr.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

intrinsic_function_expr::intrinsic_function_expr(std::string name_, std::vector<expression> args_)
: name_{std::move(name_)}, args_(std::move(args_))
{
}

const std::string& intrinsic_function_expr::name() const
{
    return name_;
}

const std::vector<expression>& intrinsic_function_expr::args() const
{
    return args_;
}

std::vector<expression> intrinsic_function_expr::sub_expressions() const
{
    return args_;
}

expression intrinsic_function_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    return intrinsic_function_expr(name_, subexprs);
}

annotation_map& intrinsic_function_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& intrinsic_function_expr::annotations()
{
    return annotations_;
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
}