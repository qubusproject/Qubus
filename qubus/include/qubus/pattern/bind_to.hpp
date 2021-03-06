#ifndef QUBUS_PATTERN_BIND_TO_HPP
#define QUBUS_PATTERN_BIND_TO_HPP

#include <qubus/pattern/variable.hpp>

#include <qubus/util/function_traits.hpp>

#include <type_traits>

namespace qubus
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
        variable<T> temp_var;

        bool result = bound_pattern_.match(value, &temp_var);

        if (result)
        {
            var_.set(temp_var.get());

            if (var)
            {
                var->set(var_.get());
            }
        }

        return result;
    }

    void reset() const
    {
        bound_pattern_.reset();
        var_.reset();
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

#endif