#ifndef QBB_UTIL_META_CONTAINS_HPP
#define QBB_UTIL_META_CONTAINS_HPP

#include <type_traits>

namespace qbb
{
namespace util
{
namespace meta
{
    
template <typename Seq, typename T>
struct contains;

template <typename Head, typename... Tail, typename T>
struct contains<type_sequence<Head, Tail...>, T> : contains<type_sequence<Tail...>, T>
{
};

template <typename... Tail, typename T>
struct contains<type_sequence<T, Tail...>, T> : std::true_type
{
};

template <typename T>
struct contains<type_sequence<>, T> : std::false_type
{
};

}
}
}

#endif