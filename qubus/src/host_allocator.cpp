#include <qbb/qubus/host_allocator.hpp>

#include <cstdlib>

namespace qbb
{
namespace qubus
{

class host_memory_block : public memory_block
{
public:
    explicit host_memory_block(void* data_, std::size_t size_, host_allocator& allocator_);

    virtual ~host_memory_block();

    std::size_t size() const override;

    void* ptr() const override;

private:
    void* data_;
    std::size_t size_;
    host_allocator* allocator_;
};

std::unique_ptr<memory_block> host_allocator::allocate(std::size_t size, std::size_t alignment)
{
    void* data;

    if (posix_memalign(&data, alignment, size))
    {
        throw std::bad_alloc();
    }

    return std::make_unique<host_memory_block>(data, size, *this);
}

void host_allocator::deallocate(memory_block& mem_block)
{
    std::free(mem_block.ptr());
}

host_memory_block::host_memory_block(void* data_, std::size_t size_, host_allocator& allocator_)
: data_(data_), size_(size_), allocator_(&allocator_)
{
}

host_memory_block::~host_memory_block()
{
    allocator_->deallocate(*this);
}

std::size_t host_memory_block::size() const
{
    return size_;
}

void* host_memory_block::ptr() const
{
    return data_;
}
}
}