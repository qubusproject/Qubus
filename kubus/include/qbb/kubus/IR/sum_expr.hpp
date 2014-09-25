#ifndef QBB_KUBUS_SUM_EXPR_HPP
#define QBB_KUBUS_SUM_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{
 
class sum_expr
{
public:
    sum_expr(expression body_, expression index_);
    sum_expr(expression body_, std::vector<expression> indices_);
    
    expression body() const;
    
    const std::vector<expression>& indices() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    expression body_;
    std::vector<expression> indices_;
    
    mutable annotation_map annotations_;
};
    
}
}

#endif