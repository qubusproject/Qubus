#ifndef QBB_KUBUS_TENSOR_VARIABLE_HPP
#define QBB_KUBUS_TENSOR_VARIABLE_HPP

#include <qbb/util/handle.hpp>
#include <qbb/kubus/IR/type.hpp>

namespace qbb
{
namespace kubus
{
 
class tensor_variable
{
public:
    tensor_variable(qbb::util::handle handle_, type tensor_type_);
    
    const qbb::util::handle& data_handle() const;
    const type& tensor_type() const;
    
private:
    qbb::util::handle handle_;
    type tensor_type_;
};

}
}


#endif