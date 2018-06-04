#ifndef QUBUS_ARRAY_HPP
#define QUBUS_ARRAY_HPP

#include <qubus/architecture_identifier.hpp>
#include <qubus/associated_qubus_type.hpp>
#include <qubus/get_view.hpp>
#include <qubus/host_object_views.hpp>
#include <qubus/object.hpp>
#include <qubus/runtime.hpp>

#include <qubus/util/integers.hpp>

#include <type_traits>

namespace qubus
{

template <typename T, util::index_t Rank>
class array
{
public:
    using value_type = T;

    template <typename... SizeTypes>
    explicit array(SizeTypes... sizes_)
    : data_(get_runtime().get_object_factory().create_array(associated_qubus_type<T>::get(),
                                                            {util::to_uindex(sizes_)...}))
    {
    }

    object get_object()
    {
        return data_;
    }

private:
    object data_;
};

template <typename T, util::index_t Rank, typename AccessType>
struct get_view_type<array<T, Rank>, AccessType, arch::host_type>
{
    using type = std::conditional_t<std::is_same_v<AccessType, detail::immutable_tag>,
                                    host_array_view<const T, Rank>, host_array_view<T, Rank>>;
};

template <typename T, long int Rank, typename AccessType, typename Arch>
auto get_view(const array<T, Rank>& value, AccessType access_type, Arch arch)
{
    return get_view<array<T, Rank>>(value.get_object(), access_type, arch);
}

} // namespace qubus

#endif
