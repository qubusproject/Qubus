#ifndef QBB_UTIL_OBSERVER_PTR_HPP
#define QBB_UTIL_OBSERVER_PTR_HPP

inline namespace qbb
{
namespace util
{

template <typename T>
class observer_ptr
{
public:
    using element_type = T;

    observer_ptr() = default;

    constexpr observer_ptr(std::nullptr_t) : ptr_(nullptr)
    {
    }

    explicit observer_ptr(element_type* ptr_) : ptr_(ptr_)
    {
    }

    observer_ptr(const observer_ptr<T>& other) = default;
    observer_ptr(observer_ptr<T>&& other) = default;

    template <typename U>
    observer_ptr(observer_ptr<U> other) : ptr_(other.ptr_)
    {
    }

    constexpr element_type* release() noexcept
    {
        T* old_ptr = ptr_;

        ptr_ = nullptr;

        return old_ptr;
    }

    constexpr void reset(element_type* ptr = nullptr) noexcept
    {
        ptr_ = ptr;
    }

    element_type* get() const noexcept
    {
        return ptr_;
    }

    element_type* operator->() const noexcept
    {
        return ptr_;
    }

    element_type& operator*() const
    {
        return *ptr_;
    }

    explicit operator bool() const
    {
        return static_cast<bool>(ptr_);
    }

    constexpr explicit operator element_type*() const
    {
        return get();
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & ptr_;
    }
private:
    template <typename U>
    friend class observer_ptr;

    element_type* ptr_;
};

template<typename T1, typename T2>
bool operator==(const observer_ptr<T1>& lhs, const observer_ptr<T1>& rhs)
{
    return lhs.get() == rhs.get();
}

template<typename T1, typename T2>
bool operator!=(const observer_ptr<T1>& lhs, const observer_ptr<T1>& rhs)
{
    return !(lhs.get() == rhs.get());
}

template<typename T1, typename T2>
bool operator<(const observer_ptr<T1>& lhs, const observer_ptr<T1>& rhs)
{
    using CT = typename std::common_type<T1*, T2*>::type;

    return std::less<CT>()(lhs.get(), rhs.get());
}

template<typename T1, typename T2>
bool operator<=(const observer_ptr<T1>& lhs, const observer_ptr<T1>& rhs)
{
    return !(rhs < lhs);
}

template<typename T1, typename T2>
bool operator>(const observer_ptr<T1>& lhs, const observer_ptr<T1>& rhs)
{
    return rhs < lhs;
}

template<typename T1, typename T2>
bool operator>=(const observer_ptr<T1>& lhs, const observer_ptr<T1>& rhs)
{
    return !(lhs < rhs);
}

template<typename T1, typename T2>
bool operator==(const observer_ptr<T1>& lhs, std::nullptr_t)
{
    return !lhs;
}

template<typename T>
bool operator==(std::nullptr_t, const observer_ptr<T>& rhs)
{
    return !rhs;
}

template<typename T>
bool operator!=(const observer_ptr<T>& lhs, std::nullptr_t)
{
    return static_cast<bool>(lhs);
}

template<typename T>
bool operator!=(std::nullptr_t, const observer_ptr<T>& rhs)
{
    return static_cast<bool>(rhs);
}

template<typename T>
bool operator<(const observer_ptr<T>& lhs, std::nullptr_t)
{
    return std::less<T*>()(lhs.get(), nullptr);
}

template<typename T>
bool operator<(std::nullptr_t, const observer_ptr<T>& rhs)
{
    return std::less<T*>()(nullptr, rhs.get());
}

template<typename T>
bool operator<=(const observer_ptr<T>& lhs, std::nullptr_t)
{
    return nullptr < lhs;
}

template<typename T>
bool operator<=(std::nullptr_t, const observer_ptr<T>& rhs)
{
    return rhs < nullptr;
}

template<typename T>
bool operator>(const observer_ptr<T>& lhs, std::nullptr_t)
{
    return !(nullptr < lhs);
}

template<typename T>
bool operator>(std::nullptr_t, const observer_ptr<T>& rhs)
{
    return !(rhs < nullptr);
}

template<typename T>
bool operator>=(const observer_ptr<T>& lhs, std::nullptr_t)
{
    return !(lhs < nullptr);
}

template<typename T>
bool operator>=(std::nullptr_t, const observer_ptr<T>& rhs)
{
    return !(nullptr < rhs);
}

template <typename T>
observer_ptr<T> make_observer(T* ptr) noexcept
{
    return observer_ptr<T>(ptr);
}

}
}

namespace std
{

template<typename T>
struct hash<qbb::util::observer_ptr<T>>
{
    using argument_type = qbb::util::observer_ptr<T>;
    using result_type = size_t;

    size_t operator()(const qbb::util::observer_ptr<T>& value) const
    {
        return std::hash<T*>()(value.get());
    }
};

}

#endif
