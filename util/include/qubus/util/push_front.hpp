#ifndef QUBUS_UTIL_PUSH_FRONT_HPP
#define QUBUS_UTIL_PUSH_FRONT_HPP

#include <qubus/util/integer_sequence.hpp>

#include <array>

namespace qubus
{
namespace util
{

namespace detail
{
template <typename T, std::size_t N, std::size_t... Indices>
inline std::array<T, N + 1> push_front_impl(const std::array<T, N>& seq, const T& value,
                                           index_sequence<Indices...>)
{
    std::array<T, N + 1> new_array = {{value, seq[Indices]...}};

    return new_array;
}
}

template <typename T, std::size_t N>
inline std::array<T, N + 1> push_front(const std::array<T, N>& seq, const T& value)
{
    return detail::push_front_impl(seq, value, make_index_sequence<N>());
} 

}
}

#endif