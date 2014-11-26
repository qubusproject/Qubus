#ifndef QBB_KUBUS_FOR_EXPR_HPP
#define QBB_KUBUS_FOR_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/variable_declaration.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{

class for_expr
{
public:
    for_expr(variable_declaration loop_index_, expression lower_bound_, expression upper_bound_, expression body_);
    for_expr(variable_declaration loop_index_, expression lower_bound_, expression upper_bound_, expression increment_, expression body_);
    
    expression body() const;

    const variable_declaration& loop_index() const;

    expression lower_bound() const;
    expression upper_bound() const;
    expression increment() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

private:
    variable_declaration loop_index_;
    expression lower_bound_;
    expression upper_bound_;
    expression increment_;
    expression body_;

    mutable annotation_map annotations_;
};

bool operator==(const for_expr& lhs, const for_expr& rhs);
bool operator!=(const for_expr& lhs, const for_expr& rhs);

template<>
struct is_expression<for_expr> : std::true_type
{
};

}
}

#endif