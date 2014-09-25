#include <qbb/kubus/IR/tensor_variable.hpp>

namespace qbb
{
namespace kubus
{

tensor_variable::tensor_variable(qbb::util::handle handle_, type tensor_type_)
: handle_{handle_}, tensor_type_{tensor_type_}
{
}

const qbb::util::handle& tensor_variable::data_handle() const
{
    return handle_;
}

const type& tensor_variable::tensor_type() const
{
    return tensor_type_;
}
}
}