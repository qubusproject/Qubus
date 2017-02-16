#ifndef QBB_QUBUS_GET_VIEW_HPP
#define QBB_QUBUS_GET_VIEW_HPP

#include <qbb/qubus/host_object_views.hpp>

#include <qbb/qubus/object.hpp>

#include <hpx/include/lcos.hpp>

#include <utility>

inline namespace qbb
{
namespace qubus
{

template <typename View>
hpx::future<View> get_view(object obj)
{
    return View::construct(std::move(obj));
}
}
}

#endif
