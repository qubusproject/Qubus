#ifndef QBB_KUBUS_COMPOUND_EXPR_HPP
#define QBB_KUBUS_COMPOUND_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{
 
class compound_expr
{
public:
    compound_expr(std::vector<expression> body_);
    
    const std::vector<expression>& body() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    std::vector<expression> body_;
    
    mutable annotation_map annotations_;
};

}
}

#endif