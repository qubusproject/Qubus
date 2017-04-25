#ifndef QUBUS_BACKENDS_CUDA_CUDA_MEMORY_BLOCK_HPP
#define QUBUS_BACKENDS_CUDA_CUDA_MEMORY_BLOCK_HPP

#include <qubus/backends/cuda/cuda_allocator.hpp>
#include <qubus/memory_block.hpp>

#include <cstddef>

namespace qubus
{

class cuda_memory_block : public memory_block
{
public:
    explicit cuda_memory_block(void* data_, std::size_t size_, cuda_allocator& allocator_);

    virtual ~cuda_memory_block();

    std::size_t size() const override;

    void* ptr() const override;

private:
    void* data_;
    std::size_t size_;
    cuda_allocator* allocator_;
};

}

#endif
