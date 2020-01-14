#include <qubus/backends/cuda/cuda_allocator.hpp>

#include <qubus/backends/cuda/cuda_memory_block.hpp>

#include <cstdlib>
#include <cstring>
#include <memory>

namespace qubus
{
cuda_allocator::cuda_allocator(cuda::context& ctx_) : ctx_(&ctx_)
{
}

cuda_memory_block cuda_allocator::allocate(std::size_t size)
{
    try
    {
        cuda::context_guard guard(*ctx_);

        auto ptr = cuda::device_malloc(size);

        return cuda_memory_block(ptr, size, *this);
    }
    catch (const cuda::cuda_error&)
    {
        throw std::bad_alloc();
    }
}

void cuda_allocator::deallocate(cuda_memory_block& mem_block)
{
    cuda::context_guard guard(*ctx_);

    auto ptr = mem_block.ptr();

    cuda::device_free(ptr);
}
}
