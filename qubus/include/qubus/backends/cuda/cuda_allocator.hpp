#ifndef QUBUS_BACKENDS_CUDA_CUDA_ALLOCATOR_HPP
#define QUBUS_BACKENDS_CUDA_CUDA_ALLOCATOR_HPP

#include <qubus/backends/cuda/cuda_memory_block.hpp>

#include <qubus/cuda/core.hpp>

#include <cstddef>
#include <memory>

namespace qubus
{

class cuda_allocator
{
public:
    using memory_block_type = cuda_memory_block;

    explicit cuda_allocator(cuda::context& ctx_);

    cuda_memory_block allocate(std::size_t size);

    void deallocate(cuda_memory_block& mem_block);

private:
    cuda::context* ctx_;
};

} // namespace qubus

#endif
