#ifndef QUBUS_HOST_MEMORY_HPP
#define QUBUS_HOST_MEMORY_HPP

#include <qubus/allocator.hpp>
#include <qubus/architecture_identifier.hpp>

#include <cstddef>

namespace qubus
{

class host_memory_block
{
public:
    host_memory_block(std::size_t size_, std::size_t alignment_);

    ~host_memory_block();

    host_memory_block(const host_memory_block& other) = delete;
    host_memory_block& operator=(const host_memory_block& other) = delete;

    host_memory_block(host_memory_block&&) = default;
    host_memory_block& operator=(host_memory_block&&) = default;

    std::byte* ptr() const
    {
        return memory_;
    }

    std::size_t size() const
    {
        return size_;
    }

    std::size_t alignment() const
    {
        return alignment_;
    }
private:
    std::byte* memory_;
    std::size_t size_;
    std::size_t alignment_;
};

class host_allocator
{
public:
    using memory_block_type = host_memory_block;

    host_memory_block allocate(std::size_t size, std::size_t alignment);
};

template <>
struct allocator_traits<host_allocator> : allocator_traits<>
{
    static constexpr bool has_alignment_support = true;
};

host_memory_block copy(const host_memory_block& other);

}

#endif
