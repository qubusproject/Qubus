#ifndef QBB_UTIL_ALIGN_HPP
#define QBB_UTIL_ALIGN_HPP

#include <cstddef>

namespace qubus
{
namespace util
{
inline void* align(std::size_t align, std::size_t size, void*& ptr, std::size_t& space) noexcept
{
    const std::size_t diff = align - reinterpret_cast<uintptr_t>(ptr) % align;
    if (diff + size >= space)
        return nullptr;
    else
    {
        space -= diff;
        ptr = static_cast<char*>(ptr) + diff;
        return ptr;
    }
}
}
}

#endif