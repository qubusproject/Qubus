#ifndef QUBUS_EVICTING_ALLOCATOR_HPP
#define QUBUS_EVICTING_ALLOCATOR_HPP

#include <qubus/allocator.hpp>
#include <qubus/memory_block.hpp>

#include <cstddef>
#include <memory>
#include <functional>

namespace qubus
{

class evicting_allocator : public allocator
{
public:
    evicting_allocator(std::unique_ptr<allocator> underlying_allocator_, std::function<bool(std::size_t)> evict_callback_);
    virtual ~evicting_allocator() = default;

    std::unique_ptr<memory_block> allocate(std::size_t size, std::size_t alignment) override;
    void deallocate(memory_block& mem_block) override;

private:
    std::unique_ptr<allocator> underlying_allocator_;
    std::function<bool(std::size_t)> evict_callback_;
};

}


#endif