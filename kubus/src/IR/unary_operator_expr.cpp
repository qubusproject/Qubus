#include <qbb/kubus/IR/unary_operator_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

unary_operator_expr::unary_operator_expr(unary_op_tag tag_, expression arg_)
: tag_{tag_}, arg_{std::move(arg_)}
{
}

unary_op_tag unary_operator_expr::tag() const
{
    return tag_;
}

expression unary_operator_expr::arg() const
{
    return arg_;
}

annotation_map& unary_operator_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& unary_operator_expr::annotations()
{
    return annotations_;
}

}
}