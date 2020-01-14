#ifndef QUBUS_BLOCK_ADDRESS_ALLOCATOR_HPP
#define QUBUS_BLOCK_ADDRESS_ALLOCATOR_HPP

#include <hpx/config.hpp>

#include <qubus/address_allocator.hpp>

#include <hpx/include/components.hpp>

#include <qubus/util/unused.hpp>

#include <atomic>
#include <limits>
#include <utility>

namespace qubus
{

class address_block
{
public:
    address_block() = default;

    address_block(object_id::prefix_type prefix_, object_id::suffix_type start_,
                  object_id::prefix_type end_);

    bool empty() const;

    object_id get_next_id();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar & prefix_;
        ar & start_;
        ar & end_;
    }
private:
    object_id::prefix_type prefix_ = 0;

    object_id::suffix_type start_ = 0;
    object_id::prefix_type end_ = 0;
};

class global_block_pool_server : public hpx::components::component_base<global_block_pool_server>
{
public:
    address_block acquire_new_block();

    HPX_DEFINE_COMPONENT_ACTION(global_block_pool_server, acquire_new_block,
                                acquire_new_block_action);

private:
    static_assert(std::atomic<object_id::prefix_type>::is_always_lock_free,
                  "The prefix counter should be lock-free.");

    // Note: Start with one as the zero prefix designates locally resolved ids.
    std::atomic<object_id::prefix_type> next_prefix_ = 1;
};

class global_block_pool
: public hpx::components::client_base<global_block_pool, global_block_pool_server>
{
public:
    using base_type = hpx::components::client_base<global_block_pool, global_block_pool_server>;

    global_block_pool() = default;

    global_block_pool(hpx::id_type id);
    global_block_pool(hpx::future<hpx::id_type>&& id);

    address_block acquire_new_block();
};

class block_address_allocator final : public address_allocator
{
public:
    explicit block_address_allocator(global_block_pool block_pool_);

    object_id acquire_new_id() override;

private:
    mutable hpx::lcos::local::mutex block_mutex_;

    address_block current_block_;

    global_block_pool block_pool_;
};

} // namespace qubus

#endif
