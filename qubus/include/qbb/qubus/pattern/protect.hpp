#ifndef QUBUS_PATTERN_PROTECT_HPP
#define QUBUS_PATTERN_PROTECT_HPP

#include <qbb/qubus/pattern/variable.hpp>

namespace qubus
{
namespace pattern
{
template<typename Pattern>
class protect_pattern
{
public:
    explicit protect_pattern(Pattern pattern_)
    : pattern_(pattern_)
    {
        
    }
    
    template <typename BaseType>
    bool match(const BaseType& value) const
    {
        return pattern_.match(value);
    }
    
    template <typename BaseType, typename T>
    bool match(const BaseType& value, const variable<T>* var) const
    {
        return pattern_.match(value, var);
    }
    
    void reset() const
    {
    }
private:
    Pattern pattern_;
};

template<typename Pattern>
inline protect_pattern<Pattern> protect(Pattern pattern)
{
    return protect_pattern<Pattern>(pattern);
}
 
}
}

#endif