#ifndef QBB_PATTERN_PATTERN_TRAITS_HPP_HPP
#define QBB_PATTERN_PATTERN_TRAITS_HPP_HPP

#include <utility>
#include <type_traits>

inline namespace qbb
{
namespace qubus
{
namespace pattern
{

template<typename Pattern>
struct pattern_traits
{
    using pattern_type = Pattern;

    template<typename ArgType>
    using match_type = decltype(&Pattern::template match<decltype(std::declval<ArgType>()[0])>);

    template<typename ArgType>
    using value_type = typename std::remove_pointer<typename util::function_traits<match_type<ArgType>>::template arg<1>::type>::type::value_type;
};

}
}
}

#endif
