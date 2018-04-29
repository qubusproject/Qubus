#ifndef QUBUS_PATTERN_PATTERN_TRAITS_HPP_HPP
#define QUBUS_PATTERN_PATTERN_TRAITS_HPP_HPP

#include <iterator>
#include <type_traits>
#include <utility>

namespace qubus
{
namespace pattern
{

template <typename Pattern>
struct pattern_traits
{
    using pattern_type = Pattern;
};

} // namespace pattern
} // namespace qubus

#endif
