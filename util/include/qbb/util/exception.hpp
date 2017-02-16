#ifndef QBB_UTIL_EXCEPTION_HPP
#define QBB_UTIL_EXCEPTION_HPP

#include <system_error>

namespace qubus
{
namespace util
{

class exception : public std::system_error
{
public:
    explicit exception(const std::string& what)
    : std::system_error(std::error_code(), what)
    {
    }
    
    virtual ~exception() = default;
};
    
}
}


#endif