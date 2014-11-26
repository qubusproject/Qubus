#ifndef QBB_KUBUS_LOOP_OPTIMIZER_HPP
#define QBB_KUBUS_LOOP_OPTIMIZER_HPP

#include <qbb/kubus/IR/function_declaration.hpp>

namespace qbb
{
namespace kubus
{
 
function_declaration optimize_loops(const function_declaration& expr);
    
}   
}


#endif