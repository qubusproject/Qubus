#ifndef QUBUS_BACKENDS_CUDA_CUDA_ALLOCATOR_HPP
#define QUBUS_BACKENDS_CUDA_CUDA_ALLOCATOR_HPP

#include <qubus/allocator.hpp>

#include <qubus/cuda/core.hpp>

#include <memory>
#include <cstddef>

namespace qubus
{

class cuda_allocator : public allocator
{
public:
    explicit cuda_allocator(cuda::context& ctx_);

    std::unique_ptr<memory_block> allocate(std::size_t size, std::size_t alignment);

    void deallocate(memory_block& mem_block);

private:
    cuda::context* ctx_;
};
}

#endif
