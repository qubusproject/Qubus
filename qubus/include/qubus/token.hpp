#ifndef QUBUS_TOKEN_HPP
#define QUBUS_TOKEN_HPP

#include <qubus/distributed_future.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/serialization.hpp>

#include <qubus/util/unused.hpp>

#include <tuple>

namespace qubus
{

class token_server : public hpx::components::component_base<token_server>
{
public:
    token_server() = default;
    explicit token_server(distributed_future<void> is_valid_);

    ~token_server();

    hpx::id_type when_valid() const;
    hpx::id_type when_expired() const;

    HPX_DEFINE_COMPONENT_ACTION(token_server, when_valid, when_valid_action);
    HPX_DEFINE_COMPONENT_ACTION(token_server, when_expired, when_expired_action);
private:
    void expire();

    distributed_future<void> is_valid_;
    distributed_promise<void> promise_;
};

class token : public hpx::components::client_base<token, token_server>
{
public:
    using base_type = hpx::components::client_base<token, token_server>;

    token() = default;
    explicit token(hpx::shared_future<void> is_valid_);
    explicit token(distributed_future<void> is_valid_);

    token(hpx::id_type&& id);
    token(hpx::future<hpx::id_type>&& id);

    distributed_future<void> when_valid() const;
    distributed_future<void> when_expired() const;

    void release();
};

}

#endif
