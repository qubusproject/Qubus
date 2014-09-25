#ifndef KUBUS_MANAGED_MEMORY_BLOCK_HPP
#define KUBUS_MANAGED_MEMORY_BLOCK_HPP

#include <qbb/kubus/memory_block.hpp>
#include <qbb/kubus/allocator.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

class managed_memory_block
{
public:
    managed_memory_block(memory_block mem_block_, allocator_view allocator_)
    : mem_block_{std::move(mem_block_)}, allocator_{std::move(allocator_)}
    {
    }

    ~managed_memory_block()
    {
        if (mem_block_)
        {
            allocator_.deallocate(mem_block_);
        }
    }

    managed_memory_block(managed_memory_block&& other)
    : mem_block_{std::move(other.mem_block_)}, allocator_{std::move(other.allocator_)}
    {
        other.mem_block_ = memory_block();
    }

    managed_memory_block(const managed_memory_block&) = delete;

    std::size_t size() const
    {
        return mem_block_.size();
    }

    memory_block& underlying_memory_block()
    {
        return mem_block_;
    }

    const memory_block& underlying_memory_block() const
    {
        return mem_block_;
    }

private:
    memory_block mem_block_;
    allocator_view allocator_;
};
}
}

#endif