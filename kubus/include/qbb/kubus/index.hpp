#ifndef KUBUS_INDEX_HPP
#define KUBUS_INDEX_HPP

#include <qbb/util/handle.hpp>

#include <cstddef>
#include <memory>
#include <ostream>

namespace qbb
{
namespace kubus
{

class index
{
public:
    constexpr index()
    : debug_name_(nullptr)
    {
    }
    
    explicit constexpr index(const char* debug_name_)
    : debug_name_(debug_name_)
    {
    }
    
    index(const index&) = delete;
    index& operator=(const index&) = delete;
    
    qbb::util::handle id() const
    {
        return qbb::util::handle_from_ptr(this);
    }
    
    const char* debug_name() const
    {
        return debug_name_;
    }
private:
    const char* debug_name_;
};

inline bool operator==(const index& lhs, const index& rhs)
{
    return lhs.id() == rhs.id();
}

inline bool operator!=(const index& lhs, const index& rhs)
{
    return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const index& idx)
{
    return os << "index( " << idx.debug_name() << " )";
}

inline qbb::util::handle id(const index& value)
{
    return value.id();
}
}
}

#endif