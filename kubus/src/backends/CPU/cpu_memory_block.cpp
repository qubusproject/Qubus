#include <qbb/kubus/backends/cpu_memory_block.hpp>

namespace qbb
{
namespace kubus
{
cpu_memory_block::cpu_memory_block(void* data_, std::size_t size_, cpu_allocator& allocator_)
: data_(data_), size_(size_), allocator_(&allocator_)
{
}

cpu_memory_block::~cpu_memory_block()
{
    allocator_->deallocate(*this);
}

std::size_t cpu_memory_block::size() const
{
    return size_;
}

void* cpu_memory_block::ptr() const
{
    return data_;
}
}
}