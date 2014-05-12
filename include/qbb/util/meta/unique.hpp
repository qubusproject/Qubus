#ifndef QBB_UTIL_META_UNIQUE_HPP
#define QBB_UTIL_META_UNIQUE_HPP

#include <qbb/util/meta/type_sequence.hpp>
#include <qbb/util/meta/contains.hpp>

#include <type_traits>

namespace qbb
{
namespace util
{
namespace meta
{

template <typename Seq, typename NewSeq = type_sequence<>>
struct unique_impl;

template <typename Head, typename... Tail, typename... ResultSeq>
struct unique_impl<type_sequence<Head, Tail...>, type_sequence<ResultSeq...>>
{
    using type = typename std::conditional<
        contains<type_sequence<ResultSeq...>, Head>::value,
        typename unique_impl<type_sequence<Tail...>, type_sequence<ResultSeq...>>::type,
        typename unique_impl<type_sequence<Tail...>,
                             type_sequence<ResultSeq..., Head>>::type>::type;
};

template <typename NewSeq>
struct unique_impl<type_sequence<>, NewSeq>
{
    using type = NewSeq;
};

template <typename Seq>
using unique = typename unique_impl<Seq>::type;

}
}
}

#endif