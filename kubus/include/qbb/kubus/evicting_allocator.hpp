#ifndef QBB_KUBUS_EVICTING_ALLOCATOR_HPP
#define QBB_KUBUS_EVICTING_ALLOCATOR_HPP

#include <qbb/kubus/allocator.hpp>
#include <qbb/kubus/memory_block.hpp>

#include <cstddef>
#include <memory>

namespace qbb
{
namespace kubus
{

class local_address_space;

class evicting_allocator : public allocator
{
public:
    evicting_allocator(std::unique_ptr<allocator> underlying_allocator_, local_address_space* ospace_);
    virtual ~evicting_allocator() = default;

    std::unique_ptr<memory_block> allocate(std::size_t size, std::size_t alignment) override;
    void deallocate(memory_block& mem_block) override;

private:
    std::unique_ptr<allocator> underlying_allocator_;
    local_address_space* ospace_;
};

}       
}


#endif