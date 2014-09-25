#include <qbb/kubus/IR/tensor_access_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

tensor_access_expr::tensor_access_expr(std::shared_ptr<tensor_variable> variable_)
: variable_{variable_}
{
}

const std::shared_ptr<tensor_variable>& tensor_access_expr::variable() const
{
    return variable_;
}

annotation_map& tensor_access_expr::annotations() const
{
    return annotations_;
}

annotation_map& tensor_access_expr::annotations()
{
    return annotations_;
}
}
}