#include <qbb/kubus/IR/tensor_variable_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

tensor_variable_expr::tensor_variable_expr(qbb::util::handle handle_, type tensor_type_)
: handle_{std::move(handle_)}, tensor_type_{std::move(tensor_type_)}
{
}

qbb::util::handle tensor_variable_expr::handle() const
{
    return handle_;
}

const type& tensor_variable_expr::tensor_type() const
{
    return tensor_type_;
}

annotation_map& tensor_variable_expr::annotations() const
{
    return annotations_;
}

annotation_map& tensor_variable_expr::annotations()
{
    return annotations_;
}
}
}