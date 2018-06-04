#ifndef QUBUS_GET_VIEW_HPP
#define QUBUS_GET_VIEW_HPP

#include <qubus/architecture_identifier.hpp>
#include <qubus/host_object_views.hpp>

#include <qubus/local_runtime.hpp>
#include <qubus/runtime.hpp>

#include <qubus/IR/type.hpp>
#include <qubus/object.hpp>

#include <hpx/include/lcos.hpp>

#include <type_traits>
#include <utility>

namespace qubus
{

namespace detail
{

struct immutable_tag
{
};
struct writable_tag
{
};

} // namespace detail

constexpr detail::immutable_tag immutable = {};
constexpr detail::writable_tag writable = {};

template <typename AccessType>
[[nodiscard]] hpx::future<distributed_access_token>
acquire_access_for_view(object& obj, AccessType access_type) {
    if constexpr (std::is_same_v<AccessType, detail::immutable_tag>)
    {
        return get_runtime().acquire_read_access(obj);
    }
    else
    {
        return get_runtime().acquire_write_access(obj);
    }
}

template <typename Type, typename AccessType, typename Arch>
struct get_view_type;

template <typename Type, typename AccessType, typename Arch>
using get_view_type_t = typename get_view_type<Type, AccessType, Arch>::type;

template <typename Type, typename AccessType, typename Arch>
auto get_view(object obj, AccessType access_type, Arch arch)
{
    static_assert(is_architecture_identifier_v<Arch>, "arch is not a valid architecture.");

    auto access_token = acquire_access_for_view(obj, access_type);

    auto& address_space = get_local_runtime().get_address_space();

    using view_type = get_view_type_t<Type, AccessType, Arch>;

    return view_type::construct(std::move(obj), std::move(access_token), address_space);
}

} // namespace qubus

#endif
