#ifndef QBB_QUBUS_LOOP_OPTIMIZER_HPP
#define QBB_QUBUS_LOOP_OPTIMIZER_HPP

#include <qbb/qubus/IR/function_declaration.hpp>

inline namespace qbb
{
namespace qubus
{
 
function_declaration optimize_loops(function_declaration expr);
    
}   
}


#endif