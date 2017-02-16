#include <qbb/util/handle.hpp>

#include <ios>
#include <cstring>
#include <sstream>
#include <iomanip>

inline namespace qbb
{
namespace util
{

std::uintptr_t handle::id() const
{
    return id_;
}

std::string handle::str() const
{
    std::stringstream ss;
    
    ss << *this;
    
    return ss.str();
}

bool operator==(const handle& lhs, const handle& rhs)
{
    return lhs.id_ == rhs.id_;
}

bool operator!=(const handle& lhs, const handle& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const handle& lhs, const handle& rhs)
{
    return lhs.id_ < rhs.id_;
}

std::ostream& operator<<(std::ostream& os, const handle& value)
{
    std::ios  state(nullptr);
    state.copyfmt(os);
    
    os << "0x" << std::hex << std::setw(8) << std::setfill('0') << value.id_;
    
    os.copyfmt(state);
    
    return os;
}

handle handle_from_ptr(const void* ptr)
{
    std::uintptr_t id; 
    std::memcpy(&id, &ptr, sizeof(std::uintptr_t));
    
    return handle(id);
}

handle handle_factory::create() const
{
    return handle(next_free_id_++);
}

void handle_factory::release(handle value) const
{
    auto expected_next_value = value.id() + 1;
    
    next_free_id_.compare_exchange_strong(expected_next_value, value.id());
}

}
}
