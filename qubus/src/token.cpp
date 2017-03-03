#include <qubus/token.hpp>

#include <qubus/hpx_utils.hpp>

#include <qubus/util/assert.hpp>

#include <utility>

QUBUS_REGISTER_DISTRIBUTED_FUTURE(void);

using server_type = hpx::components::component<qubus::token_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_token_server);

using when_valid_action = qubus::token_server::when_valid_action;
HPX_REGISTER_ACTION_DECLARATION(when_valid_action, qubus_token_server_when_valid_action);
HPX_REGISTER_ACTION(when_valid_action, qubus_token_server_when_valid_action);

using when_expired_action = qubus::token_server::when_expired_action;
HPX_REGISTER_ACTION_DECLARATION(when_expired_action, qubus_token_server_when_expired_action);
HPX_REGISTER_ACTION(when_expired_action, qubus_token_server_when_expired_action);

namespace qubus
{

token_server::token_server(distributed_future<void> is_valid_) : is_valid_(std::move(is_valid_))
{
}

token_server::~token_server()
{
    expire();
}

hpx::id_type token_server::when_valid() const
{
    QUBUS_ASSERT(is_valid_.valid(), "Invalid future.");

    return is_valid_.get_id();
}

hpx::id_type token_server::when_expired() const
{
    QUBUS_ASSERT(promise_.valid(), "Invalid promise.");

    return promise_.get_future().get_id();
}

void token_server::expire()
{
    if (promise_.valid())
    {
        promise_.set_value();
        promise_.free();
    }
}

token::token(hpx::shared_future<void> is_valid_)
: token(make_distributed_future(std::move(is_valid_)))
{
}

token::token(distributed_future<void> is_valid_)
: base_type(new_here<token_server>(std::move(is_valid_)))
{
}

token::token(hpx::id_type&& id) : base_type(std::move(id))
{
}

token::token(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

distributed_future<void> token::when_valid() const
{
    return hpx::async<token_server::when_valid_action>(this->get_id());
}

distributed_future<void> token::when_expired() const
{
    return hpx::async<token_server::when_expired_action>(this->get_id());
}

void token::release()
{
    this->free();
}
}
