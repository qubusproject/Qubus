#ifndef QBB_KUBUS_SUM_EXPR_HPP
#define QBB_KUBUS_SUM_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/variable_declaration.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>

#include <boost/optional.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{

class sum_expr
{
public:
    sum_expr(variable_declaration contraction_index_, expression body_);
    sum_expr(std::vector<variable_declaration> contraction_indices_, expression body_);
    sum_expr(std::vector<variable_declaration> contraction_indices_, variable_declaration alias_, expression body_);

    expression body() const;
    
    const std::vector<variable_declaration>& contraction_indices() const;
    const boost::optional<variable_declaration>& alias() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    expression body_;
    std::vector<variable_declaration> contraction_indices_;
    boost::optional<variable_declaration> alias_;
    
    mutable annotation_map annotations_;
};

bool operator==(const sum_expr& lhs, const sum_expr& rhs);
bool operator!=(const sum_expr& lhs, const sum_expr& rhs);

template<>
struct is_expression<sum_expr> : std::true_type
{
};

}
}

#endif