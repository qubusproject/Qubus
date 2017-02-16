#ifndef QBB_UTIL_INTEGER_SEQUENCE_HPP
#define QBB_UTIL_INTEGER_SEQUENCE_HPP

inline namespace qbb
{
namespace util
{

template <typename T, T... I>
struct integer_sequence
{
};

template<std::size_t... I>
using index_sequence = integer_sequence<std::size_t, I...>;

namespace detail
{
// Glue two sets of indices together
template <typename I1, typename I2>
struct append_indices;

template <typename T, T... N1, T... N2>
struct append_indices<integer_sequence<T, N1...>, integer_sequence<T, N2...>>
{
    typedef integer_sequence<T, N1..., (sizeof...(N1) + N2)...> type;
};

template <typename T, T N>
struct make_indices;

// generate indices [0,N) in O(log(N)) time
template <std::size_t N>
struct make_indices<std::size_t, N>
    : append_indices<typename make_indices<std::size_t, N / 2>::type, typename make_indices<std::size_t, N - N / 2>::type>
{
};

template <>
struct make_indices<std::size_t, 0>
{
    typedef integer_sequence<std::size_t> type;
};

template <>
struct make_indices<std::size_t, 1>
{
    typedef integer_sequence<std::size_t, 0> type;
};
}

template <typename T, T N>
using make_integer_sequence = typename detail::make_indices<T, N>::type;

template <std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

}
}

#endif