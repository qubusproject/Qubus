#ifndef QBB_UTIL_HANDLE_HPP
#define QBB_UTIL_HANDLE_HPP

#include <boost/serialization/is_bitwise_serializable.hpp>

// Workaround for a missing definition of mpl_::true_ if using Boost 1.61.0 and following
#include <boost/version.hpp>
#if BOOST_VERSION >= 106100
#include <boost/mpl/bool.hpp>
#endif

#include <qubus/util/unused.hpp>

#include <cstdint>
#include <atomic>
#include <ostream>
#include <functional>
#include <string>

namespace qubus
{
namespace util
{ 

class handle
{
public:
    handle() = default;
    explicit constexpr handle(std::uintptr_t id_) : id_(id_)
    {
    }
    
    std::uintptr_t id() const;
    std::string str() const;
    
    friend bool operator==(const handle& lhs, const handle& rhs);

    friend bool operator<(const handle& lhs, const handle& rhs);

    friend std::ostream& operator<<(std::ostream& os, const handle& value);

    template<typename Archive>
    void serialize(Archive& ar, unsigned int QBB_UNUSED(version))
    {
        ar & id_;
    }
private:
    std::uintptr_t id_;
};

bool operator!=(const handle& lhs, const handle& rhs);

}
}

BOOST_IS_BITWISE_SERIALIZABLE(qubus::util::handle)

namespace std
{

template<>
struct hash<qubus::util::handle>
{
    using argument_type = qubus::util::handle;
    using result_type = std::size_t;
    
    result_type operator()(const argument_type& h) const
    {
        return std::hash<std::uintptr_t>()(h.id());
    }
};

}

namespace qubus
{
namespace util
{

handle handle_from_ptr(const void* ptr);

class handle_factory
{
public:
    handle_factory() = default;
    handle_factory(const handle_factory&) = delete;
    handle_factory& operator=(const handle_factory&) = delete;
    
    handle create() const;
    void release(handle h) const;
private:
    mutable std::atomic<std::uintptr_t> next_free_id_{0};
};

}
}

#endif