#ifndef QUBUS_PATTERN_VARIABLE_HPP
#define QUBUS_PATTERN_VARIABLE_HPP

#include <qubus/pattern/variable.hpp>

#include <qubus/util/optional_ref.hpp>

#include <boost/optional.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <memory>
#include <vector>
#include <iterator>
#include <functional>

namespace qubus
{
namespace pattern
{

template<typename T>
struct variable_traits
{
    using value_type = T;

    template<typename U>
    static value_type convert_value(const U& value)
    {
        return static_cast<T>(value);
    }
};

template<typename T>
struct variable_traits<T&>
{
    using value_type = std::reference_wrapper<T>;

    template<typename U>
    static value_type convert_value(U& value)
    {
        return static_cast<T&>(value);
    }
};

template<typename T>
struct variable_traits<std::vector<T>>
{
    using value_type = std::vector<T>;

    template<typename U>
    static value_type convert_value(const U& value)
    {
        std::vector<T> result;

        boost::copy(value, std::back_inserter(result));

        return result;
    }
};

template<typename T>
struct variable_traits<boost::optional<std::reference_wrapper<T>>>
{
    using value_type = boost::optional<std::reference_wrapper<T>>;

    template<typename U>
    static value_type convert_value(const U& value)
    {
        return static_cast<boost::optional<std::reference_wrapper<T>>>(value);
    }

    template<typename U>
    static value_type convert_value(const util::optional_ref<U>& value)
    {
        if (value)
        {
            return static_cast<boost::optional<std::reference_wrapper<T>>>(*value);
        }
        else
        {
            return boost::none;
        }
    }
};

template <typename T>
class variable
{
public:
    using value_type = T;
    using storage_type = typename variable_traits<T>::value_type;
    
    variable() : value_(std::make_shared<boost::optional<storage_type>>())
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<T>* var = nullptr) const
    {
        auto casted_value = variable_traits<T>::template convert_value(value);

        if (*value_)
        {
            if (**value_ != casted_value)
            {
                return false;
            }
        }

        value_->emplace(std::move(casted_value));

        if (var)
        {
            var->set(value);
        }

        return true;
    }

    template<typename U>
    void set(U&& value) const
    {
        value_->emplace(variable_traits<T>::template convert_value(std::forward<U>(value)));
    }

    const T& get() const
    {
        return value_->value();
    }
    
    bool is_valid() const
    {
        return *value_;
    }

    void reset() const
    {
        *value_ = boost::none;
    }
private:
    std::shared_ptr<boost::optional<storage_type>> value_;
};
}
}

#endif