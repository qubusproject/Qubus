#ifndef QUBUS_LOOP_OPTIMIZER_HPP
#define QUBUS_LOOP_OPTIMIZER_HPP

#include <qubus/IR/function.hpp>

namespace qubus
{
 
std::unique_ptr<function> optimize_loops(const function& expr);
    
}


#endif