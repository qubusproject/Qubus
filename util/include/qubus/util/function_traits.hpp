#ifndef QBB_UTIL_FUNCTION_TRAITS_HPP
#define QBB_UTIL_FUNCTION_TRAITS_HPP

#include <type_traits>
#include <tuple>

namespace qubus
{
namespace util
{

template <typename T>
struct function_traits : public function_traits<decltype(&T::operator())>
{
};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType (*)(Args...)> : public function_traits<ReturnType(Args...)>
{
};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(Args...)>
{
    static constexpr auto arity = sizeof...(Args);

    typedef ReturnType result_type;

    template <std::size_t i>
    struct arg
    {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
    };
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const>
    : public function_traits<ReturnType(Args...)>
{
};

template <typename F, std::size_t i>
using arg_type = typename function_traits<F>::template arg<i>::type;

}
}

#endif