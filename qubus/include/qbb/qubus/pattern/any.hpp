#ifndef QBB_QUBUS_PATTERN_ANY_HPP
#define QBB_QUBUS_PATTERN_ANY_HPP

#include <qbb/qubus/pattern/variable.hpp>

namespace qbb
{
namespace qubus
{
namespace pattern
{
class any
{
public:
    template <typename BaseType>
    bool match(const BaseType& value, const variable<BaseType>* var = nullptr) const
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
}

#endif