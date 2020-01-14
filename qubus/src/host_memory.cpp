#include <qubus/host_memory.hpp>

#include <qubus/util/assert.hpp>

#include <cstdlib>
#include <type_traits>

namespace qubus
{

namespace
{

template<typename T>
constexpr bool is_power_of_two(T x)
{
    static_assert(std::is_integral<T>::value, "T needs to be integral.");
    static_assert(std::is_unsigned<T>::value, "T must be unsigned.");

    return !(x == 0) && !(x & (x - 1));
}

template<typename T>
constexpr bool is_multiple_of(T x, T y)
{
    static_assert(std::is_integral<T>::value, "T needs to be integral.");

    return x % y == 0;
}

std::byte* allocate_aligned(std::size_t size, std::size_t alignment)
{
    QUBUS_ASSERT(is_multiple_of(alignment, sizeof(void*)), "The alignment needs to be a multiple of sizeof(void*).");
    QUBUS_ASSERT(is_power_of_two(alignment), "The alignment needs to be a power of two.");

    std::size_t adjusted_size = size + size % alignment;

    QUBUS_ASSERT(adjusted_size % alignment == 0, "adjusted_size needs to be a multiple of alignment.");

    void* addr = std::aligned_alloc(alignment, adjusted_size);

    QUBUS_ASSERT(addr != nullptr, "Invalid allocation.");

    return static_cast<std::byte*>(addr);
}

}

host_memory_block::host_memory_block(std::size_t size_, std::size_t alignment_)
: memory_(allocate_aligned(size_, alignment_)), size_(size_), alignment_(alignment_)
{
}

host_memory_block::~host_memory_block()
{
    std::free(memory_);
}

host_memory_block host_allocator::allocate(std::size_t size, std::size_t alignment)
{
    return host_memory_block(size, alignment);
}

host_memory_block copy(const host_memory_block& other)
{
    host_memory_block copyied_block(other.size(), other.alignment());

    QUBUS_ASSERT(copyied_block.size() == other.size(), "Block sizes are no matching.");

    std::memcpy(copyied_block.ptr(), other.ptr(), copyied_block.size());

    return copyied_block;
}

}