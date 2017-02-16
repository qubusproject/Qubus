#ifndef QBB_UTIL_BISECTION_HPP
#define QBB_UTIL_BISECTION_HPP

#include <qubus/util/integers.hpp>

#include <cmath>

namespace qubus
{
namespace util
{
namespace numeric
{
 
template<typename F, typename Real>
Real bisection(F f, Real a, Real b, Real tolerance, index_t max_iter = 20000)
{
    using std::copysign;
    
    for (index_t i = 0; i <= max_iter; ++i)
    {
        auto midpoint = (a + b)/2;
        
        auto value_at_midpoint = f(midpoint);
        
        if (value_at_midpoint == 0 || (b - a)/2 < tolerance)
        {
            return midpoint;
        }
        
        if (copysign(1, value_at_midpoint) == copysign(1, f(a)))
        {
            a = midpoint;
        }
        else
        {
            b = midpoint;
        }
    }
    
    throw 0;
}

}   
}
}

#endif