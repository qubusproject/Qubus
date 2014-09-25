#ifndef QBB_KUBUS_BINARY_OPERATOR_EXPR_HPP
#define QBB_KUBUS_BINARY_OPERATOR_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>

namespace qbb
{
namespace kubus
{

enum class binary_op_tag
{ plus,
  minus,
  multiplies,
  divides,
  assign,
  plus_assign,
  equal_to,
  not_equal_to,
  less,
  greater,
  less_equal,
  greater_equal,
  logical_and,
  logical_or };

class binary_operator_expr
{
public:
    binary_operator_expr(binary_op_tag tag_, expression left_, expression right_);

    binary_op_tag tag() const;

    expression left() const;
    expression right() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    binary_op_tag tag_;
    expression left_;
    expression right_;
    
    mutable annotation_map annotations_;
};
}
}

#endif