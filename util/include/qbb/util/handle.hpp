#ifndef QBB_UTIL_HANDLE_HPP
#define QBB_UTIL_HANDLE_HPP

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

private:
    std::uintptr_t id_;
};

}
}

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