#ifndef QBB_KUBUS_PATTERN_VARIABLE_HPP
#define QBB_KUBUS_PATTERN_VARIABLE_HPP

#include <qbb/kubus/pattern/variable.hpp>

#include <boost/optional.hpp>

#include <memory>

namespace qbb
{
namespace kubus
{
namespace pattern
{
template <typename T>
class variable
{
public:
    variable() : value_{std::make_shared<boost::optional<T>>()}
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<T>* var = nullptr) const
    {
        if (*value_)
        {
            if (**value_ != value)
            {
                return false;
            }
        }

        set(value);

        if (var)
        {
            var->set(value);
        }

        return true;
    }

    void set(const T& value) const
    {
        *value_ = value;
    }

    const T& get() const
    {
        return value_->value();
    }

private:
    std::shared_ptr<boost::optional<T>> value_;
};
}
}
}

#endif