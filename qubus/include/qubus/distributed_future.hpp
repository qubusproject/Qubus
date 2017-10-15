#ifndef QUBUS_DISTRIBUTED_FUTURE_HPP
#define QUBUS_DISTRIBUTED_FUTURE_HPP

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <boost/optional.hpp>

#include <qubus/util/assert.hpp>

#include <exception>
#include <mutex>
#include <utility>

namespace qubus
{

template <typename T>
class distributed_future_state
    : public hpx::components::abstract_component_base<distributed_future_state<T>>
{
public:
    distributed_future_state() = default;
    virtual ~distributed_future_state() = default;

    distributed_future_state(const distributed_future_state&) = delete;
    distributed_future_state& operator=(const distributed_future_state&) = delete;

    distributed_future_state(distributed_future_state&&) = delete;
    distributed_future_state& operator=(distributed_future_state&&) = delete;

    virtual T get() = 0;
    virtual void wait() const = 0;

    T get_nonvirt()
    {
        return get();
    }

    void wait_nonvirt() const
    {
        wait();
    }

    HPX_DEFINE_COMPONENT_ACTION(distributed_future_state, get_nonvirt, get_action);
    HPX_DEFINE_COMPONENT_ACTION(distributed_future_state, wait_nonvirt, wait_action);
};

template <>
class distributed_future_state<void>
    : public hpx::components::abstract_component_base<distributed_future_state<void>>
{
public:
    distributed_future_state() = default;
    virtual ~distributed_future_state() = default;

    distributed_future_state(const distributed_future_state&) = delete;
    distributed_future_state& operator=(const distributed_future_state&) = delete;

    distributed_future_state(distributed_future_state&&) = delete;
    distributed_future_state& operator=(distributed_future_state&&) = delete;

    virtual void get() = 0;
    virtual void wait() const = 0;

    void get_nonvirt()
    {
        get();
    }

    void wait_nonvirt() const
    {
        wait();
    }

    HPX_DEFINE_COMPONENT_ACTION(distributed_future_state, get_nonvirt, get_action);
    HPX_DEFINE_COMPONENT_ACTION(distributed_future_state, wait_nonvirt, wait_action);
};

template <typename T>
class distributed_future_forwarder
    : public distributed_future_state<T>,
      public hpx::components::component_base<distributed_future_forwarder<T>>
{
public:
    distributed_future_forwarder() = default;

    explicit distributed_future_forwarder(hpx::future<T> forwarded_future_) : forwarded_future_(std::move(forwarded_future_))
    {
    }

    T get()
    {
        if (forwarded_future_.valid())
            return forwarded_future_.get();

        throw 0;
    }

    void wait() const
    {
        if (forwarded_future_.valid())
        {
            forwarded_future_.wait();
        }
    }

private:
    hpx::future<T> forwarded_future_;
};

template <>
class distributed_future_forwarder<void>
    : public distributed_future_state<void>,
      public hpx::components::component_base<distributed_future_forwarder<void>>
{
public:
    distributed_future_forwarder() = default;

    explicit distributed_future_forwarder(hpx::future<void> forwarded_future_) : forwarded_future_(std::move(forwarded_future_))
    {
    }

    void get()
    {
        if (forwarded_future_.valid())
        {
            forwarded_future_.get();
        }
        else
        {
            throw 0;
        }
    }

    void wait() const
    {
        if (forwarded_future_.valid())
        {
            forwarded_future_.wait();
        }
    }

private:
    hpx::future<void> forwarded_future_;
};

template <typename T>
class distributed_shared_future_forwarder
    : public distributed_future_state<T>,
      public hpx::components::component_base<distributed_shared_future_forwarder<T>>
{
public:
    distributed_shared_future_forwarder() = default;

    explicit distributed_shared_future_forwarder(hpx::shared_future<T> forwarded_future_) : forwarded_future_(std::move(forwarded_future_))
    {
    }

    T get()
    {
        QUBUS_ASSERT(forwarded_future_.valid(), "Invalid future.");

        return forwarded_future_.get();
    }

    void wait() const
    {
        QUBUS_ASSERT(forwarded_future_.valid(), "Invalid future.");

        forwarded_future_.wait();
    }

private:
    hpx::shared_future<T> forwarded_future_;
};

template <>
class distributed_shared_future_forwarder<void>
    : public distributed_future_state<void>,
      public hpx::components::component_base<distributed_shared_future_forwarder<void>>
{
public:
    distributed_shared_future_forwarder() = default;

    explicit distributed_shared_future_forwarder(hpx::shared_future<void> forwarded_future_) : forwarded_future_(std::move(forwarded_future_))
    {
    }

    void get()
    {
        QUBUS_ASSERT(forwarded_future_.valid(), "Invalid future.");

        forwarded_future_.get();
    }

    void wait() const
    {
        QUBUS_ASSERT(forwarded_future_.valid(), "Invalid future.");

        forwarded_future_.wait();
    }

private:
    hpx::shared_future<void> forwarded_future_;
};

template <typename T>
class distributed_promise_state
    : public distributed_future_state<T>,
      public hpx::components::component_base<distributed_promise_state<T>>
{
public:
    T get()
    {
        wait();

        return *data_;
    }

    void wait() const
    {
        std::unique_lock<hpx::lcos::local::mutex> lock(mutex_);

        cv_.wait(lock, [this] { return static_cast<bool>(data_); });
    }

    void set_value(T value)
    {
        {
            std::lock_guard<hpx::lcos::local::mutex> lock(mutex_);

            data_ = std::move(value);
        }

        cv_.notify_all();
    }

    HPX_DEFINE_COMPONENT_ACTION(distributed_promise_state, set_value, set_value_action);

private:
    boost::optional<T> data_;

    mutable hpx::lcos::local::mutex mutex_;
    mutable hpx::lcos::local::condition_variable cv_;
};

template <>
class distributed_promise_state<void>
    : public distributed_future_state<void>,
      public hpx::components::component_base<distributed_promise_state<void>>
{
public:
    void get()
    {
        wait();
    }

    void wait() const
    {
        std::unique_lock<hpx::lcos::local::mutex> lock(mutex_);

        cv_.wait(lock, [this] { return data_; });
    }

    void set_value()
    {
        {
            std::lock_guard<hpx::lcos::local::mutex> lock(mutex_);

            data_ = true;
        }

        cv_.notify_all();
    }

    HPX_DEFINE_COMPONENT_ACTION(distributed_promise_state, set_value, set_value_action);

private:
    bool data_;

    mutable hpx::lcos::local::mutex mutex_;
    mutable hpx::lcos::local::condition_variable cv_;
};

template <typename T>
class distributed_future
    : public hpx::components::client_base<distributed_future<T>, distributed_future_state<T>>
{
public:
    using base_type =
        hpx::components::client_base<distributed_future<T>, distributed_future_state<T>>;

    distributed_future() = default;

    distributed_future(hpx::id_type&& id) : base_type(std::move(id))
    {
    }

    distributed_future(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
    {
    }

    T get()
    {
        QUBUS_ASSERT(this->valid(), "The object is in an invalid state.");

        return typename distributed_future_state<T>::get_action()(this->get_id());
    }

    void wait() const
    {
        QUBUS_ASSERT(this->valid(), "The object is in an invalid state.");

        typename distributed_future_state<T>::wait_action()(this->get_id());
    }

    template <typename F>
    auto then(F f) const
    {
        return hpx::async(
            [](F f, distributed_future<T> fut) {
                fut.wait();

                return f(std::move(fut));
            },
            std::move(f), *this);
    }
};

template <>
class distributed_future<void>
    : public hpx::components::client_base<distributed_future<void>, distributed_future_state<void>>
{
public:
    using base_type =
        hpx::components::client_base<distributed_future<void>, distributed_future_state<void>>;

    distributed_future() = default;

    distributed_future(hpx::id_type&& id) : base_type(std::move(id))
    {
    }

    distributed_future(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
    {
    }

    void get()
    {
        QUBUS_ASSERT(this->valid(), "The object is in an invalid state.");

        return distributed_future_state<void>::get_action()(this->get_id());
    }

    void wait() const
    {
        QUBUS_ASSERT(this->valid(), "The object is in an invalid state.");

        distributed_future_state<void>::wait_action()(this->get_id());
    }

    template <typename F>
    auto then(F f) const
    {
        return hpx::async(
            [](F f, distributed_future<void> fut) {
                fut.wait();

                return f(std::move(fut));
            },
            std::move(f), *this);
    }
};

template <typename T>
class distributed_promise
    : public hpx::components::client_base<distributed_promise<T>, distributed_promise_state<T>>
{
public:
    using base_type =
    hpx::components::client_base<distributed_promise<T>, distributed_promise_state<T>>;

    distributed_promise() : base_type(hpx::local_new<distributed_promise<T>>())
    {
    }

    distributed_future<T> get_future() const
    {
        QUBUS_ASSERT(this->valid(), "The object is in an invalid state.");

        return distributed_future<T>(hpx::id_type(this->get_id()));
    }

    void set_value(T value)
    {
        QUBUS_ASSERT(this->valid(), "The object is in an invalid state.");

        typename distributed_promise_state<T>::set_value_action()(this->get_id(), std::move(value));
    }

    // TODO: Implement set_exception
};

template <>
class distributed_promise<void>
    : public hpx::components::client_base<distributed_promise<void>,
                                          distributed_promise_state<void>>
{
public:
    using base_type =
        hpx::components::client_base<distributed_promise<void>, distributed_promise_state<void>>;

    distributed_promise() : base_type(hpx::local_new<distributed_promise_state<void>>())
    {
    }

    distributed_future<void> get_future() const
    {
        QUBUS_ASSERT(this->valid(), "The object is in an invalid state.");

        return distributed_future<void>(hpx::id_type(this->get_id()));
    }

    void set_value()
    {
        QUBUS_ASSERT(this->valid(), "The object is in an invalid state.");

        distributed_promise_state<void>::set_value_action()(this->get_id());
    }
};

template <typename T>
auto make_distributed_future(hpx::future<T> future)
{
    using distributed_future_type = distributed_future_forwarder<T>;

    QUBUS_ASSERT(future.valid(), "Invalid future.");

    return distributed_future<T>(hpx::local_new<distributed_future_type>(std::move(future)));
}

template <typename T>
auto make_distributed_future(hpx::shared_future<T> future)
{
    using distributed_future_type = distributed_shared_future_forwarder<T>;

    QUBUS_ASSERT(future.valid(), "Invalid future.");

    return distributed_future<T>(hpx::local_new<distributed_future_type>(std::move(future)));
}

template <typename T>
hpx::future<T> make_future(distributed_future<T> future)
{
    QUBUS_ASSERT(future.valid(), "Invalid future.");

    return hpx::async([](distributed_future<T> future) { return future.get(); }, std::move(future));
}
}

#define QUBUS_REGISTER_DISTRIBUTED_FUTURE(T)                                                       \
    HPX_DEFINE_GET_COMPONENT_TYPE(qubus::distributed_future_state<T>);                             \
    HPX_REGISTER_ACTION(qubus::distributed_future_state<T>::get_action,                            \
                        qubus_distributed_future_state_##T##_get_action);                          \
    HPX_REGISTER_ACTION(qubus::distributed_future_state<T>::wait_action,                           \
                        qubus_distributed_future_state_##T##_wait_action);                         \
    HPX_REGISTER_COMPONENT(hpx::components::component<qubus::distributed_future_forwarder<T>>,     \
                           qubus_distributed_future_forwarder_##T);                                \
    HPX_REGISTER_COMPONENT(                                                                        \
        hpx::components::component<qubus::distributed_shared_future_forwarder<T>>,                 \
        qubus_distributed_shared_future_forwarder_##T);                                            \
    HPX_REGISTER_COMPONENT(hpx::components::component<qubus::distributed_promise_state<T>>,        \
                           qubus_distributed_promise_state_##T);                                   \
    HPX_REGISTER_ACTION(qubus::distributed_promise_state<T>::set_value_action,                     \
                        qubus_distributed_promise_state_##T##_set_value_action);                   \
/**/

#endif
