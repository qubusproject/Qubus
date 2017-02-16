#ifndef QBB_UTIL_VIRTUAL_HPP
#define QBB_UTIL_VIRTUAL_HPP

#include <qbb/util/meta/type_sequence.hpp>
#include <qbb/util/integer_sequence.hpp>
#include <qbb/util/push_front.hpp>

#include <typeindex>
#include <type_traits>

namespace qubus
{
namespace util
{

template <typename T>
struct virtual_
{
};

template <typename T>
struct remove_virtual_impl
{
    using type = T;
};

template <typename T>
struct remove_virtual_impl<T const>
{
    using type = typename remove_virtual_impl<T>::type const;
};

template <typename T>
struct remove_virtual_impl<T&>
{
    using type = typename remove_virtual_impl<T>::type&;
};

template <typename T>
struct remove_virtual_impl<virtual_<T>>
{
    using type = T;
};

template <typename T>
using remove_virtual = typename remove_virtual_impl<T>::type;

template <typename T>
struct is_polymorphic_arg : std::false_type
{
};

template <typename T>
struct is_polymorphic_arg<virtual_<T>> : std::true_type
{
};

template <typename T>
struct is_polymorphic_arg<T const> : is_polymorphic_arg<T>
{
};

template <typename T>
struct is_polymorphic_arg<T&> : is_polymorphic_arg<T>
{
};

template <typename Args, typename PolymorphicArgs>
struct polymorphic_args_impl;

template <typename Head, typename... Tail, typename... PolymorphicArgs>
struct polymorphic_args_impl<meta::type_sequence<Head, Tail...>,
                             meta::type_sequence<PolymorphicArgs...>>
{
    using type = typename std::conditional<
        is_polymorphic_arg<Head>::value,
        typename polymorphic_args_impl<
            meta::type_sequence<Tail...>,
            meta::type_sequence<PolymorphicArgs...,
                                remove_virtual<typename std::decay<Head>::type>>>::type,
        typename polymorphic_args_impl<meta::type_sequence<Tail...>,
                                       meta::type_sequence<PolymorphicArgs...>>::type>::type;
};

template <typename PolymorphicArgs>
struct polymorphic_args_impl<meta::type_sequence<>, PolymorphicArgs>
{
    using type = PolymorphicArgs;
};

template <typename Args>
using polymorphic_args = typename polymorphic_args_impl<Args, meta::type_sequence<>>::type;

template <typename PolymorphicArgs, std::size_t... Indices>
inline std::array<std::type_index, PolymorphicArgs::size()>
polymorphic_args_rtti_impl(index_sequence<Indices...>)
{
    return {{typeid(typename PolymorphicArgs::template at<Indices>)...}};
}

template <typename... Args>
inline std::array<std::type_index, polymorphic_args<meta::type_sequence<Args...>>::size()>
polymorphic_args_rtti()
{
    using polymorphic_args_seq = polymorphic_args<meta::type_sequence<Args...>>;

    return polymorphic_args_rtti_impl<polymorphic_args_seq>(
        make_index_sequence<polymorphic_args_seq::size()>());
}

struct specialization_args_rtti_adl_enabler
{
};

inline std::array<std::type_index, 0>
specialization_args_rtti_impl(meta::type_sequence<>, meta::type_sequence<>,
                              specialization_args_rtti_adl_enabler)
{
    return {{}};
}

template <typename ParamsHead, typename... ParamsTail, typename ArgsHead, typename... ArgsTail>
inline auto specialization_args_rtti_impl(meta::type_sequence<ParamsHead, ParamsTail...>,
                                          meta::type_sequence<ArgsHead, ArgsTail...>,
                                          specialization_args_rtti_adl_enabler)
    -> typename std::enable_if<
          !is_polymorphic_arg<ArgsHead>::value,
          decltype(specialization_args_rtti_impl(meta::type_sequence<ParamsTail...>{},
                                                 meta::type_sequence<ArgsTail...>{},
                                                 specialization_args_rtti_adl_enabler{}))>::type
{
    return specialization_args_rtti_impl(meta::type_sequence<ParamsTail...>{},
                                         meta::type_sequence<ArgsTail...>{},
                                         specialization_args_rtti_adl_enabler{});
}

template <typename ParamsHead, typename... ParamsTail, typename ArgsHead, typename... ArgsTail>
inline auto specialization_args_rtti_impl(meta::type_sequence<ParamsHead, ParamsTail...>,
                                          meta::type_sequence<ArgsHead, ArgsTail...>,
                                          specialization_args_rtti_adl_enabler)
    -> typename std::enable_if<
          is_polymorphic_arg<ArgsHead>::value,
          decltype(push_front(specialization_args_rtti_impl(meta::type_sequence<ParamsTail...>{},
                                                            meta::type_sequence<ArgsTail...>{},
                                                            specialization_args_rtti_adl_enabler{}),
                              std::declval<std::type_index>()))>::type
{
    return push_front(specialization_args_rtti_impl(meta::type_sequence<ParamsTail...>{},
                                                    meta::type_sequence<ArgsTail...>{},
                                                    specialization_args_rtti_adl_enabler{}),
                      std::type_index(typeid(ParamsHead)));
}

template <typename Params, typename... Args>
inline auto specialization_args_rtti() -> decltype(specialization_args_rtti_impl(
    Params{}, meta::type_sequence<Args...>{}, specialization_args_rtti_adl_enabler{}))
{
    static_assert(Params::size() == sizeof...(Args), "internal error: arity mismatch");

    return specialization_args_rtti_impl(Params{}, meta::type_sequence<Args...>{},
                                         specialization_args_rtti_adl_enabler{});
}
}
}

#endif