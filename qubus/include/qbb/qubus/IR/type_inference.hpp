#ifndef QBB_QUBUS_TYPE_INFERENCE_HPP
#define QBB_QUBUS_TYPE_INFERENCE_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/type.hpp>
#include <qbb/qubus/IR/expression.hpp>

inline namespace qbb
{
namespace qubus
{
    
type typeof_(const expression& expr);
    
}
}

#endif