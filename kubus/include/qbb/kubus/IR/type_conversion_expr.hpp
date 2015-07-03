#ifndef QBB_QUBUS_TYPE_CONVERSION_EXPR_HPP
#define QBB_QUBUS_TYPE_CONVERSION_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/type.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class type_conversion_expr
{
public:
    type_conversion_expr(type target_type_, expression arg_);

    type target_type() const;

    expression arg() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    type target_type_;
    expression arg_;
    
    mutable annotation_map annotations_;
};

bool operator==(const type_conversion_expr& lhs, const type_conversion_expr& rhs);
bool operator!=(const type_conversion_expr& lhs, const type_conversion_expr& rhs);

template<>
struct is_expression<type_conversion_expr> : std::true_type
{
};

}
}

#endif