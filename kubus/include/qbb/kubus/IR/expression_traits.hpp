#ifndef QBB_KUBUS_EXPRESSION_TRAITS_HPP
#define QBB_KUBUS_EXPRESSION_TRAITS_HPP

#include <type_traits>

namespace qbb
{
namespace qubus
{
template <typename T>
struct is_expression : std::false_type
{
};

template<typename T>
struct is_type : std::false_type
{
};
}
}

#endif