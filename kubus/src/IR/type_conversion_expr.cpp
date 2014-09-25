#include <qbb/kubus/IR/type_conversion_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

type_conversion_expr::type_conversion_expr(class type target_type_, expression arg_)
: target_type_{std::move(target_type_)}, arg_{std::move(arg_)}
{
}

class type type_conversion_expr::target_type() const
{
    return target_type_;
}

expression type_conversion_expr::arg() const
{
    return arg_;
}

annotation_map& type_conversion_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& type_conversion_expr::annotations()
{
    return annotations_;
}

}
}