#ifndef KUBUS_CPU_ALLOCATOR_HPP
#define KUBUS_CPU_ALLOCATOR_HPP

#include <qbb/kubus/memory_block.hpp>

#include <memory>
#include <new>
#include <cstddef>

class cpu_memory_block
{
public:
    cpu_memory_block() = default;

    cpu_memory_block(void* data_, std::size_t size_) : data_{data_}, size_{size_}
    {
    }

    std::size_t size() const
    {
        return size_;
    }

    void* data()
    {
        return data_;
    }

    const void* data() const
    {
        return data_;
    }

    explicit operator bool() const
    {
        return !!data_;
    }

private:
    void* data_;
    std::size_t size_;
};

class memory_allocator
{
public:
    cpu_memory_block allocate(std::size_t size)
    {
        return cpu_memory_block(operator new(size, std::nothrow), size);
    }

    void deallocate(memory_block& mem_block)
    {
        auto cpu_mem_block = mem_block.as<cpu_memory_block>();

        operator delete(cpu_mem_block.data());
    }
};

#endif