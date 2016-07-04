#ifndef QBB_QUBUS_HOST_ALLOCATOR_HPP
#define QBB_QUBUS_HOST_ALLOCATOR_HPP

#include <qbb/qubus/allocator.hpp>

#include <memory>
#include <cstddef>

namespace qbb
{
namespace qubus
{

class host_allocator : public allocator
{
public:
    virtual ~host_allocator() = default;

    std::unique_ptr<memory_block> allocate(std::size_t size, std::size_t alignment);

    void deallocate(memory_block& mem_block);
};
}
}

#endif
