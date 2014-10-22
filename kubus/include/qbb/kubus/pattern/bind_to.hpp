#ifndef QBB_KUBUS_PATTERN_BIND_TO_HPP
#define QBB_KUBUS_PATTERN_BIND_TO_HPP

#include <qbb/kubus/pattern/variable.hpp>

namespace qbb
{
namespace kubus
{
namespace pattern
{
template <typename Pattern, typename T>
class bind_directive
{
public:
    bind_directive(Pattern bound_pattern_, const variable<T>& var_)
    : bound_pattern_(bound_pattern_), var_(var_)
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<T>* var = nullptr) const
    {
        bool result = bound_pattern_.match(value, &var_);

        if (var && result)
        {
            var->set(var_.get());
        }

        return result;
    }

private:
    Pattern bound_pattern_;
    variable<T> var_;
};

template <typename Pattern, typename T>
bind_directive<Pattern, T> bind_to(const Pattern& pattern, const variable<T>& var)
{
    return bind_directive<Pattern, T>(pattern, var);
};
}
}
}

#endif