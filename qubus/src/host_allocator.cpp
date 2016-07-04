#include <qbb/qubus/host_allocator.hpp>

#include <qbb/util/assert.hpp>

#include <cstdlib>
#include <type_traits>

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

namespace
{
template<typename T>
constexpr bool is_power_of_two(T x)
{
    static_assert(std::is_integral<T>::value, "T needs to be integral.");
    static_assert(std::is_unsigned<T>::value, "T must be unsigned.");

    return !(x == 0) && !(x & (x - 1));
}

template<typename T>
constexpr bool is_multiple_of(T x, T y)
{
    static_assert(std::is_integral<T>::value, "T needs to be integral.");

    return x % y == 0;
}
}

std::unique_ptr<memory_block> host_allocator::allocate(std::size_t size, std::size_t alignment)
{
    void* data;

    QBB_ASSERT(is_multiple_of(alignment, sizeof(void*)), "The alignment needs to be a multiple of sizeof(void*).");
    QBB_ASSERT(is_power_of_two(alignment), "The alignment needs to be a power of two.");

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