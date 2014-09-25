#ifndef QBB_UTIL_PUSH_BACK_HPP
#define QBB_UTIL_PUSH_BACK_HPP

#include <qbb/util/integer_sequence.hpp>

#include <array>

namespace qbb
{
namespace util
{

namespace detail
{
template <typename T, std::size_t N, std::size_t... Indices>
inline std::array<T, N + 1> push_back_impl(const std::array<T, N>& seq, const T& value,
                                           index_sequence<Indices...>)
{
    std::array<T, N + 1> new_array = {{seq[Indices]..., value}};

    return new_array;
}
}

template <typename T, std::size_t N>
inline std::array<T, N + 1> push_back(const std::array<T, N>& seq, const T& value)
{
    return detail::push_back_impl(seq, value, make_index_sequence<N>());
} 

}
}

#endif