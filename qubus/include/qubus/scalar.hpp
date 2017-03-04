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

        auto view = get_view<host_scalar_view<value_type>>(self.get_object());

        // TODO: Add this as a pending task
        view.then([value =
                       std::move(value)](hpx::future<host_scalar_view<value_type>> view) mutable {
            view.get().get() += std::move(value);
        });

        return self;
    }

    Derived& operator-=(ValueType value)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<host_scalar_view<value_type>>(self.get_object());

        // TODO: Add this as a pending task
        view.then([value =
                       std::move(value)](hpx::future<host_scalar_view<value_type>> view) mutable {
            view.get().get() -= std::move(value);
        });

        return self;
    }

    Derived& operator*=(ValueType value)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<host_scalar_view<value_type>>(self.get_object());

        // TODO: Add this as a pending task
        view.then([value =
                       std::move(value)](hpx::future<host_scalar_view<value_type>> view) mutable {
            view.get().get() *= std::move(value);
        });

        return self;
    }

    Derived& operator/=(ValueType value)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<host_scalar_view<value_type>>(self.get_object());

        // TODO: Add this as a pending task
        view.then([value =
                       std::move(value)](hpx::future<host_scalar_view<value_type>> view) mutable {
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

        auto view = get_view<host_scalar_view<value_type>>(self.get_object());

        // TODO: Add this as a pending task
        view.then(
            [](hpx::future<host_scalar_view<value_type>> view) mutable { ++view.get().get(); });

        return self;
    }

    Derived& operator++(int)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<host_scalar_view<value_type>>(self.get_object());

        // TODO: Add this as a pending task
        view.then(
            [](hpx::future<host_scalar_view<value_type>> view) mutable { view.get().get()++; });

        return self;
    }

    Derived& operator--()
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<host_scalar_view<value_type>>(self.get_object());

        // TODO: Add this as a pending task
        view.then(
            [](hpx::future<host_scalar_view<value_type>> view) mutable { --view.get().get(); });

        return self;
    }

    Derived& operator--(int)
    {
        using value_type = typename Derived::value_type;

        auto& self = static_cast<Derived&>(*this);

        auto view = get_view<host_scalar_view<value_type>>(self.get_object());

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
    : handle_(get_runtime().get_object_factory().create_scalar(associated_qubus_type<T>::get()))
    {
    }

    explicit scalar(T value_) : scalar()
    {
        auto view = get_view<host_scalar_view<T>>(handle_).get();

        view.get() = std::move(value_);
    }

    scalar<T>& operator=(T value)
    {
        auto view = get_view<host_scalar_view<T>>(handle_);

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

template <typename View, typename T>
auto get_view(const scalar<T>& value)
{
    return get_view<View>(value.get_object());
}

template <typename T>
struct associated_qubus_type<scalar<T>>
{
    static type get()
    {
        return associated_qubus_type<T>::get();
    }
};
}

#endif
