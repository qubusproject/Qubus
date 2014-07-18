#ifndef QBB_KUBUS_TENSOR_ACCESS_EXPR_HPP
#define QBB_KUBUS_TENSOR_ACCESS_EXPR_HPP

#include <qbb/kubus/IR/annotations.hpp>
#include <qbb/kubus/IR/tensor_variable.hpp>

#include <memory>

namespace qbb
{
namespace kubus
{
 
class tensor_access_expr
{
public:
    explicit tensor_access_expr(std::shared_ptr<tensor_variable> variable_);
    
    const std::shared_ptr<tensor_variable>& variable() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    std::shared_ptr<tensor_variable> variable_;
    
    mutable annotation_map annotations_;
};
    
}
}

#endif