#ifndef QBB_KUBUS_PATTERN_ANY_HPP
#define QBB_KUBUS_PATTERN_ANY_HPP

#include <qbb/kubus/pattern/variable.hpp>

namespace qbb
{
namespace kubus
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
};

static const any _ = any{}; 
}
}
}

#endif