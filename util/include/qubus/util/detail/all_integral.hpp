#ifndef QUBUS_UTIL_ALL_INTEGRAL_HPP
#define QUBUS_UTIL_ALL_INTEGRAL_HPP

#include <type_traits>

namespace qubus
{
namespace util
{
namespace detail
{

template <typename... T>
struct and_;

template <>
struct and_<> : std::true_type
{
};

template <typename Front, typename... Tail>
struct and_<Front, Tail...> : std::integral_constant<bool, Front::value && and_<Tail...>::value>
{
};

template <typename... T>
struct all_integral : and_<std::is_integral<typename std::remove_reference<T>::type>...>
{
};
}
}
}

#endif
