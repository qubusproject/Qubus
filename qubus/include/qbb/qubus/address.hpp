#ifndef QBB_QUBUS_ADDRESS_HPP
#define QBB_QUBUS_ADDRESS_HPP

#include <hpx/include/naming.hpp>

#include <functional>
#include <cstdint>
#include <array>
#include <ostream>
#include <iomanip>
#include <type_traits>

namespace qbb
{
namespace qubus
{
class address
{
public:
    constexpr address() : value_(0)
    {
    }

    constexpr address(std::uint64_t value_) : value_(value_)
    {
    }

    constexpr std::uint64_t get() const
    {
        return value_;
    }

    friend bool operator==(address lhs, address rhs)
    {
        return lhs.value_ == rhs.value_;
    }

    friend bool operator!=(address lhs, address rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator<(address lhs, address rhs)
    {
        return lhs.value_ < rhs.value_;
    }

private:
    std::uint64_t value_;
};

static_assert(sizeof(address) == sizeof(std::uint64_t),
              "The layout of qbb:qubus::address is broken.");
static_assert(std::is_standard_layout<address>::value,
              "qbb:qubus::address is not a standard layout type.");

inline std::ostream& operator<<(std::ostream& os, const address& value)
{
    std::ios state(nullptr);
    state.copyfmt(os);

    os << "0x" << std::hex << std::right << std::setfill('0') << std::setw(16) << value.get();

    os.copyfmt(state);

    return os;
}

constexpr address nulladdr = address();

}
}

namespace std
{
template <>
struct hash<qbb::qubus::address>
{
    using argument_type = qbb::qubus::address;
    using result_type = std::size_t;

    result_type operator()(const argument_type& addr) const
    {
        result_type hash = std::hash<std::uint64_t>()(addr.get());

        return hash;
    }
};
}

#endif
