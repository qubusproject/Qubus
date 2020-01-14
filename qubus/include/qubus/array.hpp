#ifndef QUBUS_ARRAY_HPP
#define QUBUS_ARRAY_HPP

#include <qubus/architecture_identifier.hpp>
#include <qubus/associated_qubus_type.hpp>
#include <qubus/get_view.hpp>
#include <qubus/host_object_views.hpp>
#include <qubus/object.hpp>
#include <qubus/runtime.hpp>
#include <qubus/scalar.hpp>

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
    explicit array(SizeTypes... sizes_) : data_(init(sizes_...))
    {
    }

    ~array()
    {
        get_runtime().destruct(data_);
    }

    object get_object() const
    {
        return data_;
    }

private:
    template <typename... SizeTypes>
    static object init(SizeTypes... sizes)
    {
        auto arg_objects = {scalar<util::index_t>(util::to_uindex(sizes))...};

        std::vector<object> args;
        args.reserve(arg_objects.size());

        for (const auto& obj : arg_objects)
        {
            args.push_back(obj.get_object());
        }

        auto value_type = associated_qubus_type<T>::get();

        return get_runtime()
            .construct(types::array(std::move(value_type), sizeof...(sizes)), std::move(args))
            .get();
    }

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
