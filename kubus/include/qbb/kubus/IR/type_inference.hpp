#ifndef QBB_QUBUS_TYPE_INFERENCE_HPP
#define QBB_QUBUS_TYPE_INFERENCE_HPP

#include <qbb/kubus/IR/type.hpp>
#include <qbb/kubus/IR/expression.hpp>

namespace qbb
{
namespace qubus
{
    
type typeof_(const expression& expr);
    
}
}

#endif