#ifndef QUBUS_OBJECT_ID_HPP
#define QUBUS_OBJECT_ID_HPP

#include <qubus/util/hash.hpp>
#include <qubus/util/unused.hpp>

#include <tuple>
#include <cstring>
#include <cstdint>
#include <functional>
#include <ostream>

namespace qubus
{

struct local_id_tag {};

constexpr local_id_tag local_id = {};

class object_id
{
public:
    using prefix_type = std::uint64_t;
    using suffix_type = std::uint64_t;

    object_id()
    : object_id(local_id, nullptr)
    {
    }

    object_id(local_id_tag, void* ptr)
    : id_msb_(0)
    {
        static_assert(sizeof(id_lsb_) == sizeof(ptr), "Unexpected size difference.");

        std::memcpy(&id_lsb_, &ptr, sizeof(id_lsb_));
    }

    constexpr object_id(std::uint64_t id_msb_, std::uint64_t id_lsb_) : id_msb_(id_msb_), id_lsb_(id_lsb_)
    {
    }

    template <typename T>
    object_id(T id_msb_, T id_lsb_) = delete;

    constexpr std::uint64_t id_msb() const
    {
        return id_msb_;
    }

    constexpr std::uint64_t id_lsb() const
    {
        return id_lsb_;
    }

    explicit operator bool() const
    {
        void* ptr;

        static_assert(sizeof(id_lsb_) == sizeof(ptr), "Unexpected size difference.");

        std::memcpy(&ptr, &id_lsb_, sizeof(ptr));

        return !(id_msb_ == 0 && ptr == nullptr);
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar & id_msb_;
        ar & id_lsb_;
    }
private:
    std::uint64_t id_msb_;
    std::uint64_t id_lsb_;
};

inline constexpr bool operator==(const object_id& lhs, const object_id& rhs)
{
    return lhs.id_msb() == rhs.id_msb() && lhs.id_lsb() == rhs.id_lsb();
}

inline constexpr bool operator!=(const object_id& lhs, const object_id& rhs)
{
    return !(lhs == rhs);
}

inline constexpr bool operator<(const object_id& lhs, const object_id& rhs)
{
    return std::make_tuple(lhs.id_msb(), lhs.id_lsb()) < std::make_tuple(rhs.id_msb(), rhs.id_lsb());
}

inline constexpr bool operator<=(const object_id& lhs, const object_id& rhs)
{
    return std::make_tuple(lhs.id_msb(), lhs.id_lsb()) <= std::make_tuple(rhs.id_msb(), rhs.id_lsb());
}

inline constexpr bool operator>(const object_id& lhs, const object_id& rhs)
{
    return std::make_tuple(lhs.id_msb(), lhs.id_lsb()) > std::make_tuple(rhs.id_msb(), rhs.id_lsb());
}

inline constexpr bool operator>=(const object_id& lhs, const object_id& rhs)
{
    return std::make_tuple(lhs.id_msb(), lhs.id_lsb()) >= std::make_tuple(rhs.id_msb(), rhs.id_lsb());
}

inline std::ostream& operator<<(std::ostream& out, object_id id)
{
    out << id.id_msb() << id.id_lsb();

    return out;
}

} // namespace qubus

namespace std
{
template <>
struct hash<qubus::object_id>
{
    using argument_type = qubus::object_id;
    using result_type = std::size_t;

    result_type operator()(const argument_type& value) const
    {
        result_type result = 0;

        qubus::util::hash_combine(result, value.id_msb());
        qubus::util::hash_combine(result, value.id_lsb());

        return result;
    }
};
} // namespace std

#endif
