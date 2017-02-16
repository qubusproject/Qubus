#ifndef QUBUS_HPX_UTILS_HPP
#define QUBUS_HPX_UTILS_HPP

#include <hpx/include/lcos.hpp>
#include <hpx/include/components.hpp>

#include <utility>

namespace qubus
{

template <typename Component, typename... Args>
hpx::future<hpx::id_type> new_here(Args&&... args)
{
    using component_type = hpx::components::component<Component>;

    return hpx::make_ready_future(
        hpx::id_type(hpx::components::server::construct<component_type>(
                         std::forward<Args>(args)...),
                     hpx::id_type::managed));
}
}

#endif
