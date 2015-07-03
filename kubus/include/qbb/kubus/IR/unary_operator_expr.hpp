#ifndef QBB_KUBUS_UNARY_OPERATOR_EXPR_HPP
#define QBB_KUBUS_UNARY_OPERATOR_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

enum class unary_op_tag
{ plus,
  negate,
  logical_not };

class unary_operator_expr
{
public:
    unary_operator_expr(unary_op_tag tag_, expression arg_);

    unary_op_tag tag() const;

    expression arg() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    unary_op_tag tag_;
    expression arg_;
    
    mutable annotation_map annotations_;
};

bool operator==(const unary_operator_expr& lhs, const unary_operator_expr& rhs);
bool operator!=(const unary_operator_expr& lhs, const unary_operator_expr& rhs);

template<>
struct is_expression<unary_operator_expr> : std::true_type
{
};

}
}

#endif