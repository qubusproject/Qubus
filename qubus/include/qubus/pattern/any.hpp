#ifndef QUBUS_PATTERN_ANY_HPP
#define QUBUS_PATTERN_ANY_HPP

#include <qubus/pattern/variable.hpp>

namespace qubus
{
namespace pattern
{
class any
{
public:
    template <typename BaseType>
    bool match(BaseType&& value, const variable<BaseType>* var = nullptr) const
    {
        if(var)
        {
            var->set(value);
        }
        
        return true;
    }
    
    void reset() const
    {
    }
};

static const any _ = any{}; 
}
}

#endif