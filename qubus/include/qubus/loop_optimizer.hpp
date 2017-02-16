#ifndef QUBUS_LOOP_OPTIMIZER_HPP
#define QUBUS_LOOP_OPTIMIZER_HPP

#include <qubus/IR/function_declaration.hpp>

namespace qubus
{
 
function_declaration optimize_loops(function_declaration expr);
    
}


#endif