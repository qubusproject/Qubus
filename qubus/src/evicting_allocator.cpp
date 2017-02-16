#include <qubus/evicting_allocator.hpp>

#include <utility>

namespace qubus
{

evicting_allocator::evicting_allocator(std::unique_ptr<allocator> underlying_allocator_, std::function<bool(std::size_t)> evict_callback_)
: underlying_allocator_(std::move(underlying_allocator_)), evict_callback_(std::move(evict_callback_))
{
}

std::unique_ptr<memory_block> evicting_allocator::allocate(std::size_t size, std::size_t alignment)
{
    auto memblock = underlying_allocator_->allocate(size, alignment);

    while (!memblock)
    {
        if (!evict_callback_(0))
            return {};

        memblock = underlying_allocator_->allocate(size, alignment);
    }

    return memblock;
}

void evicting_allocator::deallocate(memory_block& mem_block)
{
    underlying_allocator_->deallocate(mem_block);
}
}