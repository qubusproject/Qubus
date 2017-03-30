#ifndef QUBUS_QTL_KERNEL_HELPERS_HPP
#define QUBUS_QTL_KERNEL_HELPERS_HPP

#include <boost/hana/for_each.hpp>
#include <boost/hana/functional/apply.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/unpack.hpp>

#include <qubus/util/function_traits.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace qubus
{
namespace qtl
{

template <typename Kernel>
struct get_kernel_arg_type_t
{
    template <typename Index>
    constexpr auto operator()(Index index) const
    {
        return boost::hana::type_c<util::arg_type<Kernel, Index::value>>;
    }
};

template <typename Kernel>
constexpr auto get_kernel_arg_type = get_kernel_arg_type_t<Kernel>{};

struct instantiate_t
{
    template <typename Type>
    auto operator()(Type type) const
    {
        using value_type = typename Type::type;

        return value_type{};
    }
};

constexpr auto instantiate = instantiate_t{};

template <typename Kernel>
constexpr auto instantiate_kernel_args()
{
    constexpr std::size_t kernel_arity = util::function_traits<Kernel>::arity;

    constexpr auto arg_types = boost::hana::transform(
        boost::hana::to_tuple(boost::hana::range_c<std::size_t, 0, kernel_arity>),
        get_kernel_arg_type<Kernel>);

    return boost::hana::transform(arg_types, instantiate);
}

}
}

#endif
