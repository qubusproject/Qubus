#include <qubus/backends/cuda/cuda_memory_block.hpp>

namespace qubus
{
cuda_memory_block::cuda_memory_block(void* data_, std::size_t size_, cuda_allocator& allocator_)
: data_(data_), size_(size_), allocator_(&allocator_)
{
}

cuda_memory_block::~cuda_memory_block()
{
    allocator_->deallocate(*this);
}

std::size_t cuda_memory_block::size() const
{
    return size_;
}

void* cuda_memory_block::ptr() const
{
    return data_;
}
}
