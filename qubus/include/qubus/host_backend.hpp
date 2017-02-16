#ifndef QBB_HOST_BACKEND_HPP
#define QBB_HOST_BACKEND_HPP

#include <qubus/backend.hpp>
#include <qubus/local_address_space.hpp>

#include <functional>
#include <vector>

namespace qubus
{

class host_backend : public backend
{
public:
    host_backend() = default;
    virtual ~host_backend() = default;

    host_backend(const host_backend&) = delete;
    host_backend& operator=(const host_backend&) = delete;

    virtual host_address_space& get_host_address_space() = 0;
};
}

#endif
