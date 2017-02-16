#ifndef QBB_QUBUS_CPU_MEMORY_BLOCK_HPP
#define QBB_QUBUS_CPU_MEMORY_BLOCK_HPP

#include <qbb/qubus/backends/cpu_allocator.hpp>
#include <qbb/qubus/memory_block.hpp>

#include <cstddef>

namespace qubus
{

class cpu_memory_block : public memory_block
{
public:
    explicit cpu_memory_block(void* data_, std::size_t size_, cpu_allocator& allocator_);

    virtual ~cpu_memory_block();

    std::size_t size() const override;

    void* ptr() const override;

private:
    void* data_;
    std::size_t size_;
    cpu_allocator* allocator_;
};

}

#endif