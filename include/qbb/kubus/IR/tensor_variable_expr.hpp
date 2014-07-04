#ifndef QBB_KUBUS_TENSOR_VARIABLE_EXPR_HPP
#define QBB_KUBUS_TENSOR_VARIABLE_EXPR_HPP

#include <qbb/util/handle.hpp>
#include <qbb/kubus/IR/type.hpp>
#include <qbb/kubus/IR/annotations.hpp>

namespace qbb
{
namespace kubus
{
 
class tensor_variable_expr
{
public:
    explicit tensor_variable_expr(qbb::util::handle handle_, type tensor_type_);
    
    qbb::util::handle handle() const;
    const type& tensor_type() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    qbb::util::handle handle_;
    type tensor_type_;
    
    mutable annotation_map annotations_;
};
    
}
}

#endif