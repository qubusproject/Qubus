#ifndef QUBUS_UTIL_MAKE_UNIQUE_HPP
#define QUBUS_UTIL_MAKE_UNIQUE_HPP

#include <utility>
#include <memory>

namespace qubus
{
namespace util
{
    
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}
}

#endif
