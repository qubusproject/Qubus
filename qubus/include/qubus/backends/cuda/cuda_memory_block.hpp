#ifndef QUBUS_BACKENDS_CUDA_CUDA_MEMORY_BLOCK_HPP
#define QUBUS_BACKENDS_CUDA_CUDA_MEMORY_BLOCK_HPP

#include <qubus/backends/cuda/cuda_arch.hpp>
#include <qubus/cuda/core.hpp>

#include <cstddef>

namespace qubus
{

class cuda_allocator;

class cuda_memory_block
{
public:
    explicit cuda_memory_block(cuda::device_ptr data_, std::size_t size_, cuda_allocator& allocator_);

    ~cuda_memory_block();

    std::size_t size() const;

    cuda::device_ptr ptr() const;

private:
    cuda::device_ptr data_;
    std::size_t size_;
    cuda_allocator* allocator_;
};

cuda_memory_block allocate(arch::cuda_type context, std::size_t size, std::size_t alignment);

}

#endif
