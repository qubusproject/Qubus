#ifndef QUBUS_UTIL_META_TYPE_SEQUENCE_HPP
#define QUBUS_UTIL_META_TYPE_SEQUENCE_HPP

#include <tuple>

namespace qubus
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