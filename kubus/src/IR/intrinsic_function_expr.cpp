#include <qbb/kubus/IR/intrinsic_function_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
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

annotation_map& intrinsic_function_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& intrinsic_function_expr::annotations()
{
    return annotations_;
}

}
}