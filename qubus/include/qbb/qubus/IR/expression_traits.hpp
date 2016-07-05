#ifndef QBB_QUBUS_EXPRESSION_TRAITS_HPP
#define QBB_QUBUS_EXPRESSION_TRAITS_HPP

#include <type_traits>

namespace qbb
{
namespace qubus
{
template <typename T>
class expression_base
{
protected:
    ~expression_base() = default;
};

template <typename T>
struct is_expression : std::is_base_of<expression_base<T>, T>
{
};

template<typename T>
class type_base
{
protected:
    ~type_base() = default;
};

template<typename T>
struct is_type : std::is_base_of<type_base<T>, T>
{
};
}
}

#endif