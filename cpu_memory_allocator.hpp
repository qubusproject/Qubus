#ifndef KUBUS_CPU_ALLOCATOR_HPP
#define KUBUS_CPU_ALLOCATOR_HPP

#include "memory_block.hpp"

#include <memory>
#include <new>
#include <cstddef>

class cpu_memory_block : public memory_block
{
public:
    cpu_memory_block(void* data_, std::size_t size_)
    : data_{data_}, size_{size_}
    {
    }
    
    virtual ~cpu_memory_block() = default;

    std::size_t size() const override
    {
        return size_;
    }
    
    void* data()
    {
        return data_;
    }
private:
    void* data_;
    std::size_t size_;
};

class raw_delete
{
public:
    void operator()(void* ptr) const
    {
        operator delete(ptr);
    }
};

class memory_allocator
{
public:
    std::unique_ptr<cpu_memory_block> allocate(std::size_t size)
    {
        std::unique_ptr<void,raw_delete> tmp_owner {operator new(size, std::nothrow)};

        return std::unique_ptr<cpu_memory_block>(new cpu_memory_block(tmp_owner.release(),size));
    }
};

#endif