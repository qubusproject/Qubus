#ifndef QUBUS_EXPRESSION_TRAITS_HPP
#define QUBUS_EXPRESSION_TRAITS_HPP

#include <type_traits>

namespace qubus
{

template <typename T>
struct is_expression : std::true_type
{
};

}

#endif