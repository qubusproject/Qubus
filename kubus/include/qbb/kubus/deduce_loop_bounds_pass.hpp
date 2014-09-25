#ifndef QBB_KUBUS_DEDUCE_LOOP_BOUNDS_PASS_HPP
#define QBB_KUBUS_DEDUCE_LOOP_BOUNDS_PASS_HPP

#include <qbb/kubus/IR/expression.hpp>

namespace qbb
{
namespace kubus
{

expression deduce_loop_bounds(const expression& expr);
    
}   
}


#endif