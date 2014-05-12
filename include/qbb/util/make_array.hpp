#ifndef QBB_UTIL_MAKE_ARRAY_HPP
#define QBB_UTIL_MAKE_ARRAY_HPP

#include <qbb/util/push_back.hpp>

#include <array>

namespace qbb
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
        const auto& value = *first;
        
        return push_back(make_array_impl<T, N - 1>::eval(++first, last), value);
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

template <typename T, std::size_t N, typename ForwardRange>
std::array<T, N> make_array(const ForwardRange& range)
{
    return detail::make_array_impl<T, N>::eval(begin(range), end(range));
}

}
}

#endif