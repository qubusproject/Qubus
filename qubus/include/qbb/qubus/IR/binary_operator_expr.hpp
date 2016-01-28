#ifndef QBB_QUBUS_BINARY_OPERATOR_EXPR_HPP
#define QBB_QUBUS_BINARY_OPERATOR_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

enum class binary_op_tag
{ nop,
  plus,
  minus,
  multiplies,
  divides,
  modulus,
  div_floor,
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
    binary_operator_expr();
    binary_operator_expr(binary_op_tag tag_, expression left_, expression right_);

    binary_op_tag tag() const;

    expression left() const;
    expression right() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & tag_;
        ar & left_;
        ar & right_;
    }
private:
    binary_op_tag tag_;
    expression left_;
    expression right_;
    
    mutable annotation_map annotations_;
};

bool operator==(const binary_operator_expr& lhs, const binary_operator_expr& rhs);
bool operator!=(const binary_operator_expr& lhs, const binary_operator_expr& rhs);

template<>
struct is_expression<binary_operator_expr> : std::true_type
{
};

}
}

#endif