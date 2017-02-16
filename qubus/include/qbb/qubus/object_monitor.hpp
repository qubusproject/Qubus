#ifndef QBB_QUBUS_OBJECT_MONITOR_HPP
#define QBB_QUBUS_OBJECT_MONITOR_HPP

#include <qbb/qubus/token.hpp>

#include <hpx/include/lcos.hpp>

inline namespace qbb
{
namespace qubus
{

class object_monitor
{
public:
    object_monitor();

    hpx::future<token> acquire_read_access();
    hpx::future<token> acquire_write_access();
private:
    enum class access_type
    {
        read,
        write
    };

    access_type type_;
    hpx::shared_future<void> ready_;
    std::vector<hpx::future<void>> futures_;

    hpx::lcos::local::mutex acquire_mutex_;
};

}
}

#endif
