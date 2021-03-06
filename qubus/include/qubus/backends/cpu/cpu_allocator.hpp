#ifndef QUBUS_CPU_ALLOCATOR_HPP
#define QUBUS_CPU_ALLOCATOR_HPP

#include <qubus/allocator.hpp>

#include <memory>
#include <cstddef>

namespace qubus
{

class cpu_allocator : public allocator
{
public:
    virtual ~cpu_allocator() = default;

    std::unique_ptr<memory_block> allocate(std::size_t size, std::size_t alignment);

    void deallocate(memory_block& mem_block);
};
}

#endif