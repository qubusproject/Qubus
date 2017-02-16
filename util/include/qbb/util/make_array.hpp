#ifndef QBB_UTIL_MAKE_ARRAY_HPP
#define QBB_UTIL_MAKE_ARRAY_HPP

#include <qbb/util/push_back.hpp>

#include <array>

namespace qubus
{
namespace util
{

namespace detail
{

template <typename T, std::size_t N>
struct make_array_impl
{
    template <typename ForwardIterator>
    static std::array<T, N> eval(ForwardIterator first, ForwardIterator last)
    {
        const auto& value = *(--last);
        
        return push_back(make_array_impl<T, N - 1>::eval(first, last), value);
    }
};

template <typename T>
struct make_array_impl<T, 0>
{
    template <typename ForwardIterator>
    static std::array<T, 0> eval(ForwardIterator, ForwardIterator)
    {
        return {{}};
    }
};
}

template <typename T, std::size_t N, typename ForwardIterator>
std::array<T, N> make_array(const ForwardIterator& first, const ForwardIterator& last)
{
    return detail::make_array_impl<T, N>::eval(first, last);
}

template <typename T, std::size_t N, typename ForwardRange>
std::array<T, N> make_array(const ForwardRange& range)
{
    return make_array<T, N>(begin(range), end(range));
}

}
}

#endif