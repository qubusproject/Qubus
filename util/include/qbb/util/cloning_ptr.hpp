#ifndef QBB_UTIL_CLONING_PTR_HPP
#define QBB_UTIL_CLONING_PTR_HPP

#include <qbb/util/unused.hpp>

#include <functional>
#include <memory>
#include <utility>
#include <type_traits>

namespace qbb
{
namespace util
{

template <typename T>
class cloning_ptr
{
public:
    using pointer = T*;
    using element_type = T;

    cloning_ptr() = default;

    explicit cloning_ptr(T* ptr_) : ptr_(ptr_)
    {
    }

    cloning_ptr(std::unique_ptr<T>&& ptr_) : ptr_(std::move(ptr_))
    {
    }

    cloning_ptr(const cloning_ptr<T>& other) : ptr_(other.ptr_ ? other.ptr_->clone() : nullptr)
    {
    }

    cloning_ptr(cloning_ptr<T>&& other) = default;

    template <typename U>
    cloning_ptr(const cloning_ptr<U>& other) : ptr_(other.ptr_ ? other.ptr_->clone() : nullptr)
    {
    }

    template <typename U>
    cloning_ptr(cloning_ptr<U>&& other) : ptr_(std::move(other.ptr_))
    {
    }

    cloning_ptr<T>& operator=(const cloning_ptr<T>& other)
    {
        ptr_ = other.ptr_ ? other.ptr_->clone() : nullptr;

        return *this;
    }

    cloning_ptr<T>& operator=(cloning_ptr<T>&& other) = default;

    template <typename U>
    cloning_ptr<T>& operator=(const cloning_ptr<U>& other)
    {
        ptr_ = other.ptr_ ? other.ptr_->clone() : nullptr;

        return *this;
    }

    template <typename U>
    cloning_ptr<T>& operator=(cloning_ptr<U>&& other)
    {
        ptr_ = std::move(other.ptr_);

        return *this;
    }

    T* release() noexcept
    {
        return ptr_.release();
    }

    void reset(T* ptr = nullptr) noexcept
    {
        ptr_.reset(ptr);
    }

    T* get() const noexcept
    {
        return ptr_.get();
    }

    T* operator->() const noexcept
    {
        return ptr_.get();
    }

    T& operator*() const
    {
        return *ptr_;
    }

    operator std::unique_ptr<T>() &&
    {
        return std::move(ptr_);
    }

    explicit operator bool() const
    {
        return static_cast<bool>(ptr_);
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & ptr_;
    }
private:
    template <typename U>
    friend class cloning_ptr;

    std::unique_ptr<T> ptr_;
};

template<typename T1, typename T2>
bool operator==(const cloning_ptr<T1>& lhs, const cloning_ptr<T1>& rhs)
{
    return lhs.get() == rhs.get();
}

template<typename T1, typename T2>
bool operator!=(const cloning_ptr<T1>& lhs, const cloning_ptr<T1>& rhs)
{
    return !(lhs.get() == rhs.get());
}

template<typename T1, typename T2>
bool operator<(const cloning_ptr<T1>& lhs, const cloning_ptr<T1>& rhs)
{
    using CT = typename std::common_type<T1*, T2*>::type;

    return std::less<CT>()(lhs.get(), rhs.get());
}

template<typename T1, typename T2>
bool operator<=(const cloning_ptr<T1>& lhs, const cloning_ptr<T1>& rhs)
{
    return !(rhs < lhs);
}

template<typename T1, typename T2>
bool operator>(const cloning_ptr<T1>& lhs, const cloning_ptr<T1>& rhs)
{
    return rhs < lhs;
}

template<typename T1, typename T2>
bool operator>=(const cloning_ptr<T1>& lhs, const cloning_ptr<T1>& rhs)
{
    return !(lhs < rhs);
}

template<typename T1, typename T2>
bool operator==(const cloning_ptr<T1>& lhs, std::nullptr_t)
{
    return !lhs;
}

template<typename T>
bool operator==(std::nullptr_t, const cloning_ptr<T>& rhs)
{
    return !rhs;
}

template<typename T>
bool operator!=(const cloning_ptr<T>& lhs, std::nullptr_t)
{
    return static_cast<bool>(lhs);
}

template<typename T>
bool operator!=(std::nullptr_t, const cloning_ptr<T>& rhs)
{
    return static_cast<bool>(rhs);
}

template<typename T>
bool operator<(const cloning_ptr<T>& lhs, std::nullptr_t)
{
    return std::less<T*>()(lhs.get(), nullptr);
}

template<typename T>
bool operator<(std::nullptr_t, const cloning_ptr<T>& rhs)
{
    return std::less<T*>()(nullptr, rhs.get());
}

template<typename T>
bool operator<=(const cloning_ptr<T>& lhs, std::nullptr_t)
{
    return nullptr < lhs;
}

template<typename T>
bool operator<=(std::nullptr_t, const cloning_ptr<T>& rhs)
{
    return rhs < nullptr;
}

template<typename T>
bool operator>(const cloning_ptr<T>& lhs, std::nullptr_t)
{
    return !(nullptr < lhs);
}

template<typename T>
bool operator>(std::nullptr_t, const cloning_ptr<T>& rhs)
{
    return !(rhs < nullptr);
}

template<typename T>
bool operator>=(const cloning_ptr<T>& lhs, std::nullptr_t)
{
    return !(lhs < nullptr);
}

template<typename T>
bool operator>=(std::nullptr_t, const cloning_ptr<T>& rhs)
{
    return !(nullptr < rhs);
}

template <typename T, typename... Args>
cloning_ptr<T> make_cloning(Args&&... args)
{
    return cloning_ptr<T>(new T(std::forward<Args>(args)...));
}

}
}

namespace std
{

template<typename T>
struct hash<qbb::util::cloning_ptr<T>>
{
    using argument_type = qbb::util::cloning_ptr<T>;
    using result_type = size_t;

    size_t operator()(const qbb::util::cloning_ptr<T>& value) const
    {
        return std::hash<T*>()(value.get());
    }
};

}

#endif