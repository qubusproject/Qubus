#include <qubus/block_address_allocator.hpp>

#include <qubus/util/assert.hpp>

#include <mutex>

using server_type = hpx::components::component<qubus::global_block_pool_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_global_block_pool_server);

using acquire_new_block_action = qubus::global_block_pool_server::acquire_new_block_action;
HPX_REGISTER_ACTION(acquire_new_block_action,
                    qubus_global_block_pool_server_acquire_new_block_action);

namespace qubus
{

address_block::address_block(object_id::prefix_type prefix_, object_id::suffix_type start_,
                             object_id::prefix_type end_)
: prefix_(prefix_), start_(start_), end_(end_)
{
}

bool address_block::empty() const
{
    return start_ != end_;
}

object_id address_block::get_next_id()
{
    QUBUS_ASSERT(!empty(), "The block should not be empty.");

    auto new_suffix = start_++;

    return object_id(prefix_, new_suffix);
}

address_block global_block_pool_server::acquire_new_block()
{
    auto prefix = next_prefix_.fetch_add(1, std::memory_order_relaxed);

    // Return the entire address block for the allocated prefix.
    return address_block(prefix, std::numeric_limits<object_id::suffix_type>::min(),
                         std::numeric_limits<object_id::suffix_type>::max());
}

global_block_pool::global_block_pool(hpx::id_type id) : base_type(std::move(id))
{
}

global_block_pool::global_block_pool(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

address_block global_block_pool::acquire_new_block()
{
    return hpx::sync<global_block_pool_server::acquire_new_block_action>(this->get_id());
}

block_address_allocator::block_address_allocator(global_block_pool block_pool_)
: current_block_(block_pool_.acquire_new_block()), block_pool_(std::move(block_pool_))
{
}

object_id block_address_allocator::acquire_new_id()
{
    std::lock_guard<hpx::lcos::local::mutex> guard(block_mutex_);

    // Refresh the address block if it is exhausted.
    if (current_block_.empty())
    {
        current_block_ = block_pool_.acquire_new_block();
    }

    QUBUS_ASSERT(!current_block_.empty(), "Got an invalid block.");

    return current_block_.get_next_id();
}

}