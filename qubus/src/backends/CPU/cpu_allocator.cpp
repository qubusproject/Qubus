#include <qubus/backends/cpu/cpu_allocator.hpp>

#include <qubus/backends/cpu/cpu_memory_block.hpp>

#include <qubus/util/make_unique.hpp>

#include <cstdlib>

namespace qubus
{
std::unique_ptr<memory_block> cpu_allocator::allocate(std::size_t size, std::size_t alignment)
{
    void* data;

    if (posix_memalign(&data, alignment, size))
    {
        throw std::bad_alloc();
    }

    return util::make_unique<cpu_memory_block>(data, size, *this);
}

void cpu_allocator::deallocate(memory_block& mem_block)
{
    std::free(mem_block.ptr());
}
}