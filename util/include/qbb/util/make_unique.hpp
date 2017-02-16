#ifndef QBB_UTIL_MAKE_UNIQUE_HPP
#define QBB_UTIL_MAKE_UNIQUE_HPP

#include <utility>
#include <memory>

inline namespace qbb
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
