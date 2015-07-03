#include <qbb/kubus/evicting_allocator.hpp>

#include <qbb/kubus/local_address_space.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

evicting_allocator::evicting_allocator(std::unique_ptr<allocator> underlying_allocator_, local_address_space* ospace_)
: underlying_allocator_(std::move(underlying_allocator_)), ospace_(ospace_)
{
}

std::unique_ptr<memory_block> evicting_allocator::allocate(std::size_t size, std::size_t alignment)
{
    auto memblock = underlying_allocator_->allocate(size, alignment);

    while (!memblock)
    {
        if (!ospace_->evict_objects(0))
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
}