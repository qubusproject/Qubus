#ifndef QUBUS_SCALAR_HPP
#define QUBUS_SCALAR_HPP

#include <qubus/runtime.hpp>

#include <qubus/get_view.hpp>
#include <qubus/host_object_views.hpp>

#include <qubus/object.hpp>

#include <type_traits>

namespace qubus
{

template <typename Derived, typename ValueType,
          typename Enabled = std::enable_if<std::is_arithmetic<ValueType>::value>>
class arithmetic_scalar_mixin
{
public:
    Derived& operator+=(ValueType value)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<Derived>(self.get_object(), qubus::writable, qubus::arch::host);

        // TODO: Add this as a pending task
        view.then(
            [value = std::move(value)](hpx::future<host_scalar_view<value_type>> view) mutable {
                view.get().get() += std::move(value);
            });

        return self;
    }

    Derived& operator-=(ValueType value)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<Derived>(self.get_object(), qubus::writable, qubus::arch::host);

        // TODO: Add this as a pending task
        view.then(
            [value = std::move(value)](hpx::future<host_scalar_view<value_type>> view) mutable {
                view.get().get() -= std::move(value);
            });

        return self;
    }

    Derived& operator*=(ValueType value)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<Derived>(self.get_object(), qubus::writable, qubus::arch::host);

        // TODO: Add this as a pending task
        view.then(
            [value = std::move(value)](hpx::future<host_scalar_view<value_type>> view) mutable {
                view.get().get() *= std::move(value);
            });

        return self;
    }

    Derived& operator/=(ValueType value)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<Derived>(self.get_object(), qubus::writable, qubus::arch::host);

        // TODO: Add this as a pending task
        view.then(
            [value = std::move(value)](hpx::future<host_scalar_view<value_type>> view) mutable {
                view.get().get() /= std::move(value);
            });

        return self;
    }
};

template <typename Derived, typename ValueType,
          typename Enabled = std::enable_if<std::is_integral<ValueType>::value>>
class integral_scalar_mixin
{
public:
    Derived& operator++()
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<Derived>(self.get_object(), qubus::writable, qubus::arch::host);

        // TODO: Add this as a pending task
        view.then(
            [](hpx::future<host_scalar_view<value_type>> view) mutable { ++view.get().get(); });

        return self;
    }

    Derived& operator++(int)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<Derived>(self.get_object(), qubus::writable, qubus::arch::host);

        // TODO: Add this as a pending task
        view.then(
            [](hpx::future<host_scalar_view<value_type>> view) mutable { view.get().get()++; });

        return self;
    }

    Derived& operator--()
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<Derived>(self.get_object(), qubus::writable, qubus::arch::host);

        // TODO: Add this as a pending task
        view.then(
            [](hpx::future<host_scalar_view<value_type>> view) mutable { --view.get().get(); });

        return self;
    }

    Derived& operator--(int)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<Derived>(self.get_object(), qubus::writable, qubus::arch::host);

        // TODO: Add this as a pending task
        view.then(
            [](hpx::future<host_scalar_view<value_type>> view) mutable { view.get().get()--; });

        return self;
    }
};

template <typename T>
class scalar : public arithmetic_scalar_mixin<scalar<T>, T>,
               public integral_scalar_mixin<scalar<T>, T>
{
public:
    using value_type = T;

    scalar()
    : handle_(get_runtime().construct(associated_qubus_type<T>::get(), {}).get())
    {
    }

    explicit scalar(T value_) : scalar()
    {
        auto view = get_view<scalar<T>>(handle_, qubus::writable, qubus::arch::host).get();

        view.get() = std::move(value_);
    }

    ~scalar()
    {
        get_runtime().destruct(handle_);
    }

    scalar<T>& operator=(T value)
    {
        auto view = get_view<scalar<T>>(handle_, qubus::writable, qubus::arch::host);

        // TODO: Add this as a pending task
        view.then([value = std::move(value)](hpx::future<host_scalar_view<T>> view) mutable {
            view.get().get() = std::move(value);
        });

        return *this;
    }

    object get_object() const
    {
        return handle_;
    }

private:
    object handle_;
};

template <typename T, typename AccessType>
struct get_view_type<scalar<T>, AccessType, arch::host_type>
{
    using type = std::conditional_t<std::is_same_v<AccessType, detail::immutable_tag>,
                                    host_scalar_view<const T>, host_scalar_view<T>>;
};

template <typename T, typename AccessType, typename Arch>
auto get_view(const scalar<T>& value, AccessType access_type, Arch arch)
{
    return get_view<scalar<T>>(value.get_object(), access_type, arch);
}

template <typename T>
struct associated_qubus_type<scalar<T>>
{
    static type get()
    {
        return associated_qubus_type<T>::get();
    }
};
} // namespace qubus

#endif
