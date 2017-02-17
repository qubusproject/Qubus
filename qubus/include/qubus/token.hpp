#ifndef QUBUS_TOKEN_HPP
#define QUBUS_TOKEN_HPP

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/serialization.hpp>

#include <qubus/util/one_shot.hpp>

#include <tuple>

namespace qubus
{

class token_server : public hpx::components::component_base<token_server>
{
public:
    token_server() = default;

    explicit token_server(util::one_shot<void()> on_expiration_);

    ~token_server();

private:
    void expire();

    util::one_shot<void()> on_expiration_;
};

class token : public hpx::components::client_base<token, token_server>
{
public:
    using base_type = hpx::components::client_base<token, token_server>;

    token() = default;

    token(hpx::id_type id_);
    token(hpx::future<hpx::id_type>&& id_);

    ~token();

    token(const token &) = delete;

    token &operator=(const token &) = delete;

    token(token &&) = default;

    token &operator=(token &&) = default;

    void release();
};

token make_token(util::one_shot<void()> on_expiration);

std::tuple <token, hpx::future<void>> make_token_with_future();

}

#endif
