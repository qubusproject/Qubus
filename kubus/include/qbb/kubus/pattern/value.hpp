#ifndef QBB_QUBUS_PATTERN_VALUE_HPP
#define QBB_QUBUS_PATTERN_VALUE_HPP

#include <qbb/kubus/pattern/variable.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{
namespace pattern
{

template <typename T>
class value_pattern
{
public:
    value_pattern(T value_) : value_(std::move(value_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<T>* var = nullptr) const
    {
        if (value == value_)
        {

            if (var)
            {
                var->set(value_);
            }

            return true;
        }

        return false;
    }

    void reset() const
    {
    }
private:
    T value_;
};

template <typename T>
value_pattern<T> value(T value)
{
    return value_pattern<T>(value);
}
}
}
}

#endif