#ifndef QUBUS_TOKEN_HPP
#define QUBUS_TOKEN_HPP

#include <qubus/distributed_future.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/serialization.hpp>

#include <qubus/util/unused.hpp>

#include <tuple>

namespace qubus
{

class token
{
public:
    token() = default;
    explicit token(hpx::shared_future<void> is_valid_);
    explicit token(distributed_future<void> is_valid_);

     ~token();

    token(const token&) = delete;
    token& operator=(const token&) = delete;

    token(token&&) = default;
    token& operator=(token&&) = default;

    hpx::future<void> when_valid() const;
    distributed_future<void> when_expired() const;

    void release();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar & is_valid_;
        ar & promise_;

        is_valid_.free();
        promise_.free();
    }

private:
    distributed_future<void> is_valid_;
    distributed_promise<void> promise_;
};

}

#endif
