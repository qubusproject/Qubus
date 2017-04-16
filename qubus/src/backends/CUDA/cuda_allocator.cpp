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

std::unique_ptr<memory_block> cuda_allocator::allocate(std::size_t size, std::size_t alignment)
{
    try
    {
        cuda::context_guard guard(*ctx_);

        auto ptr = cuda::device_malloc(size);

        void* data;

        static_assert(sizeof(ptr) == sizeof(void*),
                      "The size of a CUDA device pointer has match the size of a host pointer.");

        std::memcpy(&data, &ptr, sizeof(void*));

        return std::make_unique<cuda_memory_block>(data, size, *this);
    }
    catch (const cuda::cuda_error&)
    {
        throw std::bad_alloc();
    }
}

void cuda_allocator::deallocate(memory_block& mem_block)
{
    cuda::context_guard guard(*ctx_);

    void* data = mem_block.ptr();

    cuda::device_ptr ptr;

    static_assert(sizeof(ptr) == sizeof(void*),
                  "The size of a CUDA device pointer has match the size of a host pointer.");

    std::memcpy(&ptr, &data, sizeof(void*));

    cuda::device_free(ptr);
}
}
