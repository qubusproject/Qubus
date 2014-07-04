#ifndef QBB_KUBUS_INDEX_EXPR_HPP
#define QBB_KUBUS_INDEX_EXPR_HPP

#include <qbb/kubus/IR/annotations.hpp>

#include <string>

namespace qbb
{
namespace kubus
{
 
class index_expr
{
public:
    explicit index_expr(std::string id_);
    
    std::string id() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    std::string id_;
    
    mutable annotation_map annotations_;
};
    
}
}

#endif