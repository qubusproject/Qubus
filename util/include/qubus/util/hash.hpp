#ifndef QUBUS_UTIL_HASH_HPP
#define QUBUS_UTIL_HASH_HPP

#include <functional>
#include <cstddef>

namespace qubus
{
namespace util
{
 
template <class T>
void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

}  
}

#endif