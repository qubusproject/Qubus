#ifndef QBB_UTIL_META_TYPE_SEQUENCE_HPP
#define QBB_UTIL_META_TYPE_SEQUENCE_HPP

#include <tuple>

inline namespace qbb
{
namespace util
{
namespace meta
{
    
template <typename... Values>
struct type_sequence
{
    template <std::size_t I>
    using at = typename std::tuple_element<I, std::tuple<Values...>>::type;

    static constexpr std::size_t size()
    {
        return sizeof...(Values);
    }
};

}
}
}

#endif