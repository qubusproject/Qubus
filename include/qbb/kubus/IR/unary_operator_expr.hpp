#ifndef QBB_KUBUS_UNARY_OPERATOR_EXPR_HPP
#define QBB_KUBUS_UNARY_OPERATOR_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>

namespace qbb
{
namespace kubus
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

    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    unary_op_tag tag_;
    expression arg_;
    
    mutable annotation_map annotations_;
};
}
}

#endif