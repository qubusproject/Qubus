#include <qubus/object_monitor.hpp>

#include <qubus/util/assert.hpp>

#include <mutex>

namespace qubus
{

object_monitor::object_monitor() : type_(access_type::write)
{
}

token object_monitor::acquire_read_access()
{
    std::lock_guard<hpx::lcos::local::mutex> guard(acquire_mutex_);

    if (type_ != access_type::read)
    {
        ready_ = hpx::shared_future<void>(hpx::when_all(std::move(futures_)));
        futures_ = std::vector<hpx::future<void>>();
        type_ = access_type::read;
    }

    token access_token(ready_);

    futures_.push_back(make_future(access_token.when_expired()));

    return access_token;
}

token object_monitor::acquire_write_access()
{
    std::lock_guard<hpx::lcos::local::mutex> guard(acquire_mutex_);

    ready_ = hpx::shared_future<void>(hpx::when_all(std::move(futures_)));
    futures_ = std::vector<hpx::future<void>>();
    type_ = access_type::write;

    token access_token(ready_);

    futures_.push_back(make_future(access_token.when_expired()));

    return access_token;
}
}