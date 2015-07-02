#ifndef QBB_UTIL_HANDLE_HPP
#define QBB_UTIL_HANDLE_HPP

#include <boost/serialization/is_bitwise_serializable.hpp>

#include <qbb/util/unused.hpp>

#include <cstdint>
#include <atomic>
#include <ostream>
#include <functional>
#include <string>

namespace qbb
{
namespace util
{ 

class handle
{
public:
    explicit handle(std::uintptr_t id_);
    
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

BOOST_IS_BITWISE_SERIALIZABLE(qbb::util::handle)

namespace std
{

template<>
struct hash<qbb::util::handle>
{
    using argument_type = qbb::util::handle;
    using result_type = std::size_t;
    
    result_type operator()(const argument_type& h) const
    {
        return std::hash<std::uintptr_t>()(h.id());
    }
};

}

namespace qbb
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