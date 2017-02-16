#ifndef QUBUS_UTIL_DELEGATE_HPP
#define QUBUS_UTIL_DELEGATE_HPP

#include <qubus/util/function_traits.hpp>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/mutex.hpp>

#include <functional>
#include <type_traits>
#include <mutex>
#include <utility>

namespace qubus
{
namespace util
{

template <typename Signature, typename SharedMutex = boost::shared_mutex>
class delegate
{
public:
    using result_type = typename function_traits<Signature>::result_type;

    template <typename F>
    explicit delegate(F default_callback_)
    : delegate_callback_(default_callback_), default_callback_(std::move(default_callback_))
    {
    }

    delegate(const delegate& other)
    : delegate_callback_(other.delegate_callback_), default_callback_(other.default_callback_)
    {
    }

    delegate(delegate&& other) noexcept : delegate_callback_(std::move(other.delegate_callback_)),
                                          default_callback_(std::move(other.default_callback_))
    {
    }

    delegate& operator=(delegate other) noexcept
    {
        this->swap(other);

        return *this;
    }

    template <typename F>
    void connect(F delegate_callback)
    {
        std::unique_lock<SharedMutex> guard(delegate_callback_mutex_);

        delegate_callback_ = std::move(delegate_callback);
    }

    void disconnect()
    {
        std::unique_lock<SharedMutex> guard(delegate_callback_mutex_);

        delegate_callback_ = default_callback_;
    }

    template <typename... Args>
    result_type operator()(Args&&... args) const
    {
        boost::shared_lock<SharedMutex> guard(delegate_callback_mutex_);

        return delegate_callback_(std::forward<Args>(args)...);
    }

    void swap(delegate<Signature, SharedMutex>& other) noexcept
    {
        std::swap(delegate_callback_, other.delegate_callback_);
        std::swap(default_callback_, other.default_callback_);
    }

private:
    std::function<Signature> delegate_callback_;
    std::function<Signature> default_callback_;
    mutable SharedMutex delegate_callback_mutex_;
};
}
}

namespace std
{

template <typename Signature, typename SharedMutex>
void swap(qubus::util::delegate<Signature, SharedMutex>& first,
          qubus::util::delegate<Signature, SharedMutex>& second) noexcept
{
    first.swap(second);
}
}

#endif
