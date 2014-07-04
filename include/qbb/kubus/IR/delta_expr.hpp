#ifndef QBB_KUBUS_DELTA_EXPR_HPP
#define QBB_KUBUS_DELTA_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{
 
class delta_expr
{
public:
    delta_expr(std::vector<expression> indices_);
    
    const std::vector<expression>& indices() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    std::vector<expression> indices_;
    
    mutable annotation_map annotations_;
};
    
}
}

#endif