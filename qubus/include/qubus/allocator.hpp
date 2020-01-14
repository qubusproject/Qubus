#ifndef QUBUS_ALLOCATOR_HPP
#define QUBUS_ALLOCATOR_HPP

#include <qubus/memory_block.hpp>

#include <memory>
#include <type_traits>

namespace qubus
{

class allocator
{
public:
    allocator() = default;
    virtual ~allocator() = default;

    allocator(const allocator&) = delete;
    allocator& operator=(const allocator&) = delete;

    virtual std::unique_ptr<memory_block> allocate(std::size_t size, std::size_t alignment) = 0;
    virtual void deallocate(memory_block& mem_block) = 0;

private:
};

template <typename Allocator = void>
struct allocator_traits
{
    using memory_block_type = typename Allocator::memory_block_type;

    static constexpr bool has_alignment_support = false;
};

template <>
struct allocator_traits<void>
{
    static constexpr bool has_alignment_support = false;
};

} // namespace qubus

#endif