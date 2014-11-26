#ifndef QBB_KUBUS_PATTERN_CONTROL_FLOW_HPP
#define QBB_KUBUS_PATTERN_CONTROL_FLOW_HPP

#include <type_traits>

namespace qbb
{
namespace kubus
{
namespace pattern
{
    
template<typename Value, typename Pattern, typename Then, typename Else>
auto if_then_else(const Value& value, const Pattern& pattern, const Then& then, const Else& else_)
 -> typename std::common_type<decltype(then()), decltype(else_())>::type
{
    if (pattern.match(value))
    {
       return then(); 
    }
    else
    {
        return else_();
    }
}

}   
}   
}

#endif