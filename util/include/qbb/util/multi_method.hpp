#ifndef QBB_UTIL_MULTI_METHOD_HPP
#define QBB_UTIL_MULTI_METHOD_HPP

#include <qbb/util/detail/dispatch_table.hpp>
#include <qbb/util/detail/virtual.hpp>
#include <qbb/util/function_traits.hpp>

#include <functional>
#include <typeindex>
#include <array>
#include <memory>
#include <utility>
#include <type_traits>

namespace qbb
{
namespace util
{

template <typename Arg, typename Param,
          typename Enabler = typename std::enable_if<!is_polymorphic_arg<Arg>::value, int>::type>
auto unwrap_arg(remove_virtual<Arg> arg) -> decltype(arg)
{
    return arg;
}

template <typename Arg, typename Param,
          typename Enabler = typename std::enable_if<is_polymorphic_arg<Arg>::value, int>::type>
auto unwrap_arg(remove_virtual<Arg> arg) -> decltype(arg.template as<Param>())
{
    return arg.template as<Param>();
}

template <typename T, std::size_t N, std::size_t... Indices>
std::array<T, N + 1> concat_impl(const T& value, const std::array<T, N>& sequence,
                                 index_sequence<Indices...>)
{
    return {{value, sequence[Indices]...}};
}

template <typename T, std::size_t N>
std::array<T, N + 1> concat(const T& value, const std::array<T, N>& sequence)
{
    return concat_impl(value, sequence, make_index_sequence<N>());
}

template <typename... Params>
struct deduce_key;

template <typename ParamsHead>
struct deduce_key<ParamsHead>
{

    template <typename Head>
    static std::array<index_t, 1> call_impl(std::true_type, const Head& head)
    {
        return {{head.tag()}};
    }

    template <typename Head>
    static std::array<index_t, 0> call_impl(std::false_type, const Head&)
    {
        return {{}};
    }

    template <typename Arg>
    static auto call(const Arg& arg)
        -> decltype(call_impl(std::integral_constant<bool, is_polymorphic_arg<ParamsHead>::value>(),
                              arg))
    {
        return call_impl(std::integral_constant<bool, is_polymorphic_arg<ParamsHead>::value>(),
                         arg);
    }
};

template <typename ParamsHead, typename... ParamsTail>
struct deduce_key<ParamsHead, ParamsTail...>
{

    template <typename Head, typename... Tail>
    static auto call_impl(std::true_type, const Head& head, const Tail&... tail)
        -> decltype(concat(head.tag(), deduce_key<ParamsTail...>::call(tail...)))
    {
        return concat(head.tag(), deduce_key<ParamsTail...>::call(tail...));
    }

    template <typename Head, typename... Tail>
    static auto call_impl(std::false_type, const Head&, const Tail&... tail)
        -> decltype(deduce_key<ParamsTail...>::call(tail...))
    {
        return deduce_key<ParamsTail...>::call(tail...);
    }

    template <typename... Args>
    static auto call(const Args&... args)
        -> decltype(call_impl(std::integral_constant<bool, is_polymorphic_arg<ParamsHead>::value>(),
                              args...))
    {
        return call_impl(std::integral_constant<bool, is_polymorphic_arg<ParamsHead>::value>(),
                         args...);
    }
};

template <typename, typename DispatchTable>
class basic_multi_method;

template <typename ReturnType, typename... Args, typename DispatchTable>
class basic_multi_method<ReturnType(Args...), DispatchTable>
{
public:
    using specialization_t = std::function<ReturnType(remove_virtual<Args>...)>;

    basic_multi_method() = default;

    explicit basic_multi_method(specialization_t fallback_)
    {
        set_fallback(std::move(fallback_));
    }

    ReturnType operator()(remove_virtual<Args>... args) const
    {
        if (!specializations_)
            throw std::bad_function_call();

        init_dispatch_table(false);

        const auto& specialization = dispatch_table_->find(deduce_key<Args...>::call(args...));

        return specialization(args...);
    }

    static constexpr std::size_t arity()
    {
        return sizeof...(Args);
    }

    static constexpr std::size_t polymorphic_arity()
    {
        return polymorphic_args<meta::type_sequence<Args...>>::size();
    }

    template <typename F>
    void add_specialization(F specialization)
    {
        static_assert(function_traits<F>::arity == arity(),
                      "The arity of the specialization must match the arity of the multimethod.");

        if (!specializations_)
            specializations_.reset(
                new std::map<std::array<std::type_index, polymorphic_arity()>, specialization_t>());

        add_specialization_impl(std::move(specialization), make_index_sequence<arity()>());

        init_dispatch_table(true);
    }

    void set_fallback(specialization_t fallback)
    {
        init_dispatch_table(false);

        dispatch_table_->set_fallback(std::move(fallback));
    }

private:
    void init_dispatch_table(bool updated_specializations) const
    {
        std::call_once(initialization_flag_, [this]()
                       {
                           dispatch_table_.reset(new DispatchTable());
                       });

        dispatch_table_->build_dispatch_table(*specializations_, updated_specializations);
    }

    template <typename F, std::size_t... Indices>
    void add_specialization_impl(F specialization, index_sequence<Indices...>)
    {
        specialization_t thunk = [=](remove_virtual<Args>... args)
        {
            return specialization(unwrap_arg<Args, arg_type<F, Indices>>(args)...);
        };

        specializations_->insert(
            {specialization_args_rtti<meta::type_sequence<arg_type<F, Indices>...>, Args...>(),
             std::move(thunk)});
    }

    mutable std::unique_ptr<DispatchTable> dispatch_table_;

    std::unique_ptr<std::map<std::array<std::type_index, polymorphic_arity()>, specialization_t>>
        specializations_;

    mutable std::once_flag initialization_flag_;
};

template <typename T>
using sparse_multi_method = basic_multi_method<T, sparse_dispatch_table<T>>;

template <typename T>
using multi_method = basic_multi_method<T, dense_dispatch_table<T>>;

class multi_method_specialization_initializer
{
public:
    template <typename T, typename DispatchTable, typename Specialazation>
    explicit multi_method_specialization_initializer(
        basic_multi_method<T, DispatchTable>& multi_method, Specialazation specialization)
    {
        multi_method.add_specialization(specialization);
    }
};

#define QBB_DEFINE_MULTI_METHOD_SPECIALIZATION_WITH_NAME(multi_method, specialization, name)       \
    ::qbb::util::multi_method_specialization_initializer qbb_mm_##name##_initializer(              \
        multi_method, specialization)

#define QBB_DEFINE_MULTI_METHOD_SPECIALIZATION(multi_method, specialization)                       \
    QBB_DEFINE_MULTI_METHOD_SPECIALIZATION_WITH_NAME(multi_method, specialization,                 \
                                                     multi_method##_##specialization)
}
}

#endif