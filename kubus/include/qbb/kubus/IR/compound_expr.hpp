#ifndef QBB_KUBUS_COMPOUND_EXPR_HPP
#define QBB_KUBUS_COMPOUND_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{
 
class compound_expr
{
public:
    compound_expr(std::vector<expression> body_);
    
    const std::vector<expression>& body() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    std::vector<expression> body_;
    
    mutable annotation_map annotations_;
};

bool operator==(const compound_expr& lhs, const compound_expr& rhs);
bool operator!=(const compound_expr& lhs, const compound_expr& rhs);

template<>
struct is_expression<compound_expr> : std::true_type
{
};

}
}

#endif