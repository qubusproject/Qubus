#include <qbb/kubus/backends/cpu_allocator.hpp>

#include <qbb/kubus/backends/cpu_memory_block.hpp>

#include <qbb/util/make_unique.hpp>

#include <cstdlib>

namespace qbb
{
namespace kubus
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
}