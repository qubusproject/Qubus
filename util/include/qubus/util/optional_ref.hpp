#ifndef QUBUS_UTIL_OPTIONAL_REF_HPP
#define QUBUS_UTIL_OPTIONAL_REF_HPP

#include <boost/optional.hpp>
#include <type_traits>

namespace qubus
{
namespace util
{

template <typename T>
class optional_ref
{
public:
    using value_type = typename std::remove_cv<T>::type;

    optional_ref() : ref_(nullptr)
    {
    }

    optional_ref(std::nullopt_t)
    : optional_ref()
    {
    }

    optional_ref(T& ref_) : ref_(&ref_)
    {
    }

    optional_ref(T* ref_) : ref_(ref_)
    {
    }

    optional_ref<T>& operator=(T& ref_)
    {
        this->ref_ = &ref_;

        return *this;
    }

    T& get() const
    {
        return *ref_;
    }

    T& operator*() const
    {
        return get();
    }

    T* operator->() const
    {
        return ref_;
    }

    explicit operator bool() const
    {
        return ref_;
    }

    explicit operator boost::optional<value_type>() const
    {
        if (ref_)
        {
            return *ref_;
        }
        else
        {
            return {};
        }
    }

private:
    T* ref_;
};

template <typename T>
bool operator==(const optional_ref<T>& lhs, const optional_ref<T>& rhs)
{
    if (!lhs && !rhs)
    {
        return true;
    }
    else if (lhs && rhs && *lhs == *rhs)
    {
        return true;
    }
    else
    {
        return false;
    }
}

template <typename T>
bool operator!=(const optional_ref<T>& lhs, const optional_ref<T>& rhs)
{
    return !(lhs == rhs);
}
}
}

#endif