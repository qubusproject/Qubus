#ifndef QBB_KUBUS_FOR_ALL_EXPR_HPP
#define QBB_KUBUS_FOR_ALL_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{
 
class for_all_expr
{
public:
    for_all_expr(expression index_, expression body_);
    
    expression body() const;
    
    expression index() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    expression index_;
    expression body_;
    
    mutable annotation_map annotations_;
};

}
}

#endif