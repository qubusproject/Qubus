#ifndef QUBUS_TYPE_INFERENCE_HPP
#define QUBUS_TYPE_INFERENCE_HPP

#include <qubus/IR/type.hpp>
#include <qubus/IR/expression.hpp>

namespace qubus
{
    
type typeof_(const expression& expr);
    
}

#endif