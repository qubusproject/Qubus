#ifndef QUBUS_GET_VIEW_HPP
#define QUBUS_GET_VIEW_HPP

#include <qubus/host_object_views.hpp>

#include <qubus/runtime.hpp>
#include <qubus/local_runtime.hpp>

#include <qubus/object.hpp>

#include <hpx/include/lcos.hpp>

#include <utility>

namespace qubus
{

template <typename View>
[[nodiscard]] hpx::future<distributed_access_token> acquire_access_for_view(object& obj)
{
    if (!object_view_traits<View>::is_immutable)
    {
        return get_runtime().acquire_write_access(obj);
    }
    else
    {
        return get_runtime().acquire_read_access(obj);
    }
}

template <typename View>
hpx::future<View> get_view(object obj)
{
    auto access_token = acquire_access_for_view<View>(obj);

    auto& address_space = get_local_runtime().get_address_space();

    return View::construct(std::move(obj), std::move(access_token), address_space);
}

}

#endif
