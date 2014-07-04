#ifndef QBB_KUBUS_SUBSCRIPTION_EXPR_HPP
#define QBB_KUBUS_SUBSCRIPTION_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{

class subscription_expr
{
public:
    subscription_expr(expression indexed_expr_, std::vector<expression> indices_);
    
    expression indexed_expr() const;
    
    const std::vector<expression>& indices() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    expression indexed_expr_;
    std::vector<expression> indices_;
    
    mutable annotation_map annotations_;
};
    
}
}


#endif