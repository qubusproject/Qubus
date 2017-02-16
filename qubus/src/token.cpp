#include <qubus/token.hpp>

#include <qubus/hpx_utils.hpp>

using server_type = hpx::components::component<qubus::token_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_token_server);

namespace qubus
{

token_server::token_server(util::one_shot<void()> on_expiration_)
: on_expiration_(std::move(on_expiration_))
{
}

token_server::~token_server()
{
    expire();
}

void token_server::expire()
{
    if (on_expiration_)
    {
        on_expiration_();
    }
}

token::token(hpx::id_type id_)
: base_type(std::move(id_))
{
}

token::token(hpx::future<hpx::id_type>&& id_)
: base_type(std::move(id_))
{
}

token::~token()
{
    release();
}

void token::release()
{
    this->free();
}

token make_token(util::one_shot<void()> on_expiration)
{
    return new_here<token_server>(std::move(on_expiration));
}

std::tuple<token, hpx::future<void>> make_token_with_future()
{
    hpx::promise<void> expiration_promise;

    hpx::future<void> expiration_future = expiration_promise.get_future();

    auto token = make_token([promise = std::move(expiration_promise)] () mutable { promise.set_value(); });

    return std::make_tuple(std::move(token), std::move(expiration_future));
}

}

