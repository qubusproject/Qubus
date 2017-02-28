#include <qubus/token.hpp>

#include <qubus/util/assert.hpp>

QUBUS_REGISTER_DISTRIBUTED_FUTURE(void);

namespace qubus
{

token::token(hpx::shared_future<void> is_valid_) : token(make_distributed_future(std::move(is_valid_)))
{
}

token::token(distributed_future<void> is_valid_) : is_valid_(std::move(is_valid_))
{
}

token::~token()
{
    release();
}

hpx::future<void> token::when_valid() const
{
    QUBUS_ASSERT(is_valid_.valid(), "Invalid future.");

    return make_future(is_valid_);
}

distributed_future<void> token::when_expired() const
{
    QUBUS_ASSERT(promise_.valid(), "Invalid promise.");

    return promise_.get_future();
}

void token::release()
{
    if (promise_.valid())
    {
        promise_.set_value();
        promise_.free();
    }
}
}
