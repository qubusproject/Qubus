#ifndef QBB_KUBUS_ALLOCATOR_HPP
#define QBB_KUBUS_ALLOCATOR_HPP

#include <qbb/kubus/memory_block.hpp>

#include <memory>

namespace qbb
{
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

}
}

#endif