#ifndef QUBUS_PATTERN_TYPE_HPP
#define QUBUS_PATTERN_TYPE_HPP

#include <qubus/IR/type.hpp>
#include <qubus/IR/types.hpp>

#include <qubus/pattern/any.hpp>
#include <qubus/pattern/sequence.hpp>
#include <qubus/pattern/value.hpp>
#include <qubus/pattern/variable.hpp>

#include <utility>

namespace qubus::pattern
{

template <typename T>
class primitive_type_pattern
{
public:
    template <typename BaseType>
    bool match(const BaseType& value, const variable<T>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<T>())
        {
            if (var)
            {
                var->set(*concret_value);
            }

            return true;
        }

        return false;
    }

    void reset() const
    {
    }
};

class integer_type_pattern
{
public:
    bool match(const type& value, const variable<type>* var = nullptr) const
    {
        if (is_integer(value))
        {
            if (var)
            {
                var->set(value);
            }

            return true;
        }

        return false;
    }

    void reset() const
    {
    }
};

constexpr integer_type_pattern integer_t = {};

}

#endif