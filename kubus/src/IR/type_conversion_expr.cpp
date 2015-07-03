#include <qbb/kubus/IR/type_conversion_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
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

std::vector<expression> type_conversion_expr::sub_expressions() const
{
    return {arg_};
}

expression type_conversion_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 1, "invalid number of subexpressions");
    
    return type_conversion_expr(target_type_, subexprs[0]);
}

annotation_map& type_conversion_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& type_conversion_expr::annotations()
{
    return annotations_;
}

bool operator==(const type_conversion_expr& lhs, const type_conversion_expr& rhs)
{
    return lhs.target_type() == rhs.target_type() && lhs.arg() == rhs.arg();
}

bool operator!=(const type_conversion_expr& lhs, const type_conversion_expr& rhs)
{
    return !(lhs == rhs);
}

}
}