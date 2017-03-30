#ifndef QUBUS_GET_VIEW_HPP
#define QUBUS_GET_VIEW_HPP

#include <qubus/host_object_views.hpp>

#include <qubus/object.hpp>

#include <hpx/include/lcos.hpp>

#include <utility>

namespace qubus
{

template <typename View>
hpx::future<View> get_view(object obj)
{
    return View::construct(std::move(obj));
}

template <typename View>
hpx::future<View> get_view_for_locked_object(object obj)
{
    return View::construct_from_locked_object(std::move(obj));
}
}

#endif
