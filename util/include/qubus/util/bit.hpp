#ifndef QUBUS_BIT_HPP
#define QUBUS_BIT_HPP

#include <cstring>
#include <type_traits>

namespace qubus
{
namespace util
{

template <class To, class From>
typename std::enable_if<(sizeof(To) == sizeof(From)) && std::is_trivially_copyable<From>::value &&
                            std::is_trivial<To>::value,
                        To>::type
bit_cast(const From& src) noexcept
{
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

} // namespace util
} // namespace qubus

#endif
