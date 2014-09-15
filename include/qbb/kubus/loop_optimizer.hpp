#ifndef QBB_KUBUS_LOOP_OPTIMIZER_HPP
#define QBB_KUBUS_LOOP_OPTIMIZER_HPP

#include <qbb/kubus/IR/expression.hpp>

namespace qbb
{
namespace kubus
{
 
expression optimize_loops(const expression& expr);
    
}   
}


#endif