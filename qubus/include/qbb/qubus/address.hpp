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
    constexpr address() : components_{{0, 0}}
    {
    }

    constexpr address(std::uint64_t msb, std::uint64_t lsb) : components_{{msb, lsb}}
    {
    }

    constexpr std::uint64_t msb() const
    {
        return components_[0];
    }

    constexpr std::uint64_t lsb() const
    {
        return components_[1];
    }

    friend bool operator==(address lhs, address rhs)
    {
        return lhs.components_ == rhs.components_;
    }

    friend bool operator!=(address lhs, address rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator<(address lhs, address rhs)
    {
        return lhs.components_ < rhs.components_;
    }

private:
    std::array<std::uint64_t, 2> components_;
};

static_assert(sizeof(address) == 2 * sizeof(std::uint64_t),
              "The layout of qbb:qubus::address is broken.");
static_assert(std::is_standard_layout<address>::value,
              "qbb:qubus::address is not a standard layout type.");

inline std::ostream& operator<<(std::ostream& os, const address& value)
{
    std::ios state(nullptr);
    state.copyfmt(os);

    os << "0x" << std::hex << std::right << std::setfill('0') << std::setw(16) << value.msb()
       << std::right << std::setfill('0') << std::setw(16) << value.lsb();

    os.copyfmt(state);

    return os;
}

constexpr address nulladdr = address();

inline address make_address_from_id(const hpx::id_type &id)
{
    return address(id.get_msb(), id.get_lsb());
}

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
        result_type h1 = std::hash<std::uint64_t>()(addr.lsb());
        result_type h2 = std::hash<std::uint64_t>()(addr.msb());
        return h1 ^ (h2 << 1);
    }
};
}

#endif
