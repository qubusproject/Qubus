#ifndef QBB_QUBUS_LOCAL_ADDRESS_SPACE_HPP
#define QBB_QUBUS_LOCAL_ADDRESS_SPACE_HPP

#include <qbb/qubus/evicting_allocator.hpp>

#include <qbb/qubus/allocator.hpp>
#include <qbb/qubus/memory_block.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/util/integers.hpp>

#include <map>
#include <vector>
#include <memory>

namespace qbb
{
namespace qubus
{
    
// TODO: remove memory blocks of dead objects
class local_address_space
{
public:
    explicit local_address_space(std::unique_ptr<allocator> allocator_);

    local_address_space(const local_address_space&) = delete;
    local_address_space& operator=(const local_address_space&) = delete;
    
    void register_mem_block(qbb::util::handle address, std::unique_ptr<memory_block> mem_block);

    // FIXME: return a future
    std::shared_ptr<memory_block> get_mem_block(const qbb::util::handle& address) const;

    allocator& get_allocator() const;
    
    void dump() const;

    bool evict_objects(std::size_t hint);
private:
    //object clone_object(const qbb::util::handle& object_handle) const;

    std::unique_ptr<evicting_allocator> allocator_;

    // FIXME: we need to protect this with a mutex
    mutable std::map<qbb::util::handle, std::shared_ptr<memory_block>> objects_;
};

}
}

#endif