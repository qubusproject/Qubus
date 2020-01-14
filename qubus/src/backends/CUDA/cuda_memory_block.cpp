#include <qubus/backends/cuda/cuda_memory_block.hpp>

namespace qubus
{
cuda_memory_block::cuda_memory_block(cuda::device_ptr data_, std::size_t size_, cuda_allocator& allocator_)
: data_(data_), size_(size_), allocator_(&allocator_)
{
}

cuda_memory_block::~cuda_memory_block()
{

}

std::size_t cuda_memory_block::size() const
{
    return size_;
}

cuda::device_ptr cuda_memory_block::ptr() const
{
    return data_;
}

cuda_memory_block allocate(arch::cuda_type context, std::size_t size, std::size_t alignment)
{
    cuda::device_malloc(size);
}
}
