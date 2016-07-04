#include <qbb/qubus/object_monitor.hpp>

#include <qbb/util/assert.hpp>

#include <mutex>

namespace qbb
{
namespace qubus
{

object_monitor::object_monitor()
: type_(access_type::write)
{
}

hpx::future<token> object_monitor::acquire_read_access()
{
    std::lock_guard<hpx::lcos::local::mutex> guard(acquire_mutex_);

    if (type_ != access_type::read)
    {
        ready_ = hpx::shared_future<void>(hpx::when_all(std::move(futures_)));
        futures_ = std::vector<hpx::future<void>>();
        type_ = access_type::read;
    }

    auto token_and_future = make_token_with_future();

    auto access_token = std::move(std::get<0>(token_and_future));

    futures_.push_back(std::move(std::get<1>(token_and_future)));

    return ready_.then([access_token = std::move(access_token)](const hpx::shared_future<void>& ready) mutable {
        ready.get();

        return std::move(access_token);
    });
}

hpx::future<token> object_monitor::acquire_write_access()
{
    std::lock_guard<hpx::lcos::local::mutex> guard(acquire_mutex_);

    ready_ = hpx::shared_future<void>(hpx::when_all(std::move(futures_)));
    futures_ = std::vector<hpx::future<void>>();
    type_ = access_type::write;

    auto token_and_future = make_token_with_future();

    auto access_token = std::move(std::get<0>(token_and_future));

    futures_.push_back(std::move(std::get<1>(token_and_future)));

    return ready_.then([access_token = std::move(access_token)](const hpx::shared_future<void>& ready) mutable {
        ready.get();

        return std::move(access_token);
    });
}

}
}