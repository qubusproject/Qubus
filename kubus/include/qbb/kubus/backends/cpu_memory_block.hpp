#ifndef QBB_KUBUS_CPU_MEMORY_BLOCK_HPP
#define QBB_KUBUS_CPU_MEMORY_BLOCK_HPP

#include <qbb/kubus/backends/cpu_allocator.hpp>
#include <qbb/kubus/memory_block.hpp>

#include <cstddef>

namespace qbb
{
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
}

#endif