#ifndef QBB_UTIL_HANDLE_HPP
#define QBB_UTIL_HANDLE_HPP

#include <cstddef>
#include <atomic>
#include <ostream>

namespace qbb
{
namespace util
{ 
    
class handle
{
public:
    friend bool operator==(const handle& lhs, const handle& rhs);

    friend bool operator<(const handle& lhs, const handle& rhs);

    friend std::ostream& operator<<(std::ostream& os, const handle& value);

private:
    friend class handle_factory;

    explicit handle(std::size_t id_);

    std::size_t id_;
};

class handle_factory
{
public:
    handle_factory() = default;
    handle_factory(const handle_factory&) = delete;
    handle_factory& operator=(const handle_factory&) = delete;
    
    handle create() const;
    void release(handle h) const;
private:
    mutable std::atomic<std::size_t> next_free_id_{0};
};

}
}

#endif