#include <qubus/local_address_space.hpp>

#include <qubus/local_runtime.hpp>

#include <qubus/object.hpp>

#include <qubus/evicting_allocator.hpp>

#include <qubus/logging.hpp>

#include <hpx/parallel/executors.hpp> // Workaround for missing includes.

#include <qubus/util/assert.hpp>
#include <qubus/util/unused.hpp>

#include <tuple>
#include <type_traits>
#include <utility>

namespace qubus
{

address_space::handle::handle(std::shared_ptr<address_entry> entry_) : entry_(std::move(entry_))
{
}

memory_block& address_space::handle::data() const
{
    return entry_->data();
}

address_space::handle::operator bool() const
{
    return static_cast<bool>(entry_);
}

bool address_space::handle::unique() const
{
    return entry_.unique();
}

void address_space::handle::free()
{
    entry_.reset();
}

address_space::page_fault_context::page_fault_context(address_space& addr_space_)
: addr_space_(&addr_space_)
{
}

address_space::handle address_space::page_fault_context::allocate_page(long int size,
                                                                       long int alignment)
{
    QUBUS_ASSERT(addr_space_, "Invalid context.");

    return addr_space_->allocate_page(size, alignment);
}

address_space::address_entry::address_entry(std::unique_ptr<memory_block> data_)
: data_(std::move(data_))
{
}

memory_block& address_space::address_entry::data() const
{
    return *data_;
}

address_space::address_space(std::unique_ptr<allocator> allocator_)
: allocator_(std::make_unique<evicting_allocator>(
      std::move(allocator_), [this](std::size_t hint) { return evict_objects(hint); })),
  on_page_fault_([](const object&, page_fault_context) {
      // TODO: Add correct exception type.
      return hpx::make_exceptional_future<address_space::handle>(
          std::runtime_error("Page fault"));
  })
{
}

address_space::handle address_space::allocate_page(long int size, long int alignment)
{
    auto data = allocator_->allocate(size, alignment);

    return handle(std::make_shared<address_entry>(std::move(data)));
}

void address_space::register_page(const object& obj, handle page)
{
    bool addr_was_free;
    using entry_table_iterator = decltype(entry_table_.begin());
    entry_table_iterator pos;

    auto addr = obj.get_id().get_gid();

    {
        std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

        std::tie(pos, addr_was_free) =
            entry_table_.emplace(addr, hpx::make_ready_future(std::move(page)));
    }

    QUBUS_ASSERT(addr_was_free, "Address is spuriously occupied.");
}

void address_space::free_object(const object& obj)
{
    std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

    entry_table_.erase(obj.get_id().get_gid());
}

hpx::future<address_space::handle> address_space::resolve_object(const object& obj)
{
    std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

    auto addr = obj.get_id().get_gid();

    auto entry = entry_table_.find(addr);

    if (entry != entry_table_.end())
    {
        // Short-circuit the future query
        if (entry->second.is_ready())
        {
            return hpx::make_ready_future(handle(entry->second.get()));
        }

        return entry->second.then(
            get_local_runtime().get_service_executor(),
            [](const hpx::shared_future<handle>& entry) { return entry.get(); });
    }

    // Unlock the mutex since on_page_fault_ might call other member functions/block/... .
    guard.unlock();

    try
    {
        hpx::future<handle> entry = on_page_fault_(obj, page_fault_context(*this));

        // Relock the mutex to add the entry to the address table.
        guard.lock();

        bool addr_was_free;
        using entry_table_iterator = decltype(entry_table_.begin());
        entry_table_iterator pos;

        std::tie(pos, addr_was_free) = entry_table_.emplace(addr, std::move(entry));

        QUBUS_ASSERT(addr_was_free, "Address is spuriously occupied.");

        return pos->second.then(
            get_local_runtime().get_service_executor(),
            [](const hpx::shared_future<handle>& entry) { return entry.get(); });
    }
    catch (...)
    {
        throw 0;
    }
}

address_space::handle address_space::try_resolve_object(const object& obj) const
{
    std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

    auto addr = obj.get_id().get_gid();

    auto entry = entry_table_.find(addr);

    if (entry != entry_table_.end())
    {
        if (entry->second.is_ready())
        {
            return handle(entry->second.get());
        }
    }

    return handle();
}

bool address_space::evict_objects(std::size_t hint)
{
    std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

    BOOST_LOG_NAMED_SCOPE("local_address_space");

    logger slg;

    std::size_t freed_memory = 0;

    for (const auto& entry : entry_table_)
    {
        if (entry.second.is_ready())
        {
            if (entry.second.get().unique())
            {
                QUBUS_LOG(slg, info) << "evicting object " << entry.first;

                freed_memory += entry.second.get().data().size();

                entry_table_.erase(entry.first);

                if (freed_memory >= hint)
                {
                    return true;
                }
            }
        }
    }

    bool has_freed_memory = freed_memory > 0;

    return has_freed_memory;
}

void address_space::on_page_fault(
    std::function<hpx::future<address_space::handle>(const object&, page_fault_context)> callback)
{
    on_page_fault_.connect(std::move(callback));
}

void address_space::dump() const
{
    std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

    /*BOOST_LOG_NAMED_SCOPE("address_space");

    logger slg;

    QUBUS_LOG(slg, info) << "address table\n";
    QUBUS_LOG(slg, info) << "contains " << address_translation_table_.size()
                         << " address mappings\n";

    for (const auto& handle_object_pair : address_translation_table_)
    {
        QUBUS_LOG(slg, info) << handle_object_pair.first << " -> "
                             << handle_object_pair.second->ptr() << "\n";
    }*/
}

local_address_space::page_fault_context::page_fault_context(
    host_address_space::page_fault_context host_ctx_)
: host_ctx_(std::move(host_ctx_))
{
}

local_address_space::handle
local_address_space::page_fault_context::allocate_page(long int size, long int alignment)
{
    return host_ctx_.allocate_page(size, alignment);
}

local_address_space::local_address_space(host_address_space& host_addr_space_)
: host_addr_space_(host_addr_space_),
  on_page_fault_(
      [](const object& QUBUS_UNUSED(obj), page_fault_context) { // TODO: Add correct exception type.
          return hpx::make_exceptional_future<local_address_space::handle>(
              std::runtime_error("Page fault"));
      })
{
    host_addr_space_.on_page_fault(
        [this](const object& obj,
               address_space::page_fault_context ctx) -> hpx::future<address_space::handle> {
            auto components = obj.components();

            if (!obj.has_data())
            {
                auto page = ctx.allocate_page(0, sizeof(void*));

                return hpx::make_ready_future(std::move(page));
            }

            return on_page_fault_(obj, page_fault_context(std::move(ctx)));
        });
}

local_address_space::handle local_address_space::allocate_page(long int size, long int alignment)
{
    return host_addr_space_.get().allocate_page(size, alignment);
}

void local_address_space::register_page(const object& obj, local_address_space::handle page)
{
    host_addr_space_.get().register_page(obj, std::move(page));
}

void local_address_space::free_object(const object& obj)
{
    host_addr_space_.get().free_object(obj);
}

hpx::future<local_address_space::handle> local_address_space::resolve_object(const object& obj)
{
    return host_addr_space_.get().resolve_object(obj);
}

local_address_space::handle local_address_space::try_resolve_object(const object& obj) const
{
    return host_addr_space_.get().try_resolve_object(obj);
}

void local_address_space::on_page_fault(
    std::function<hpx::future<local_address_space::handle>(const object& obj, page_fault_context)>
        callback)
{
    on_page_fault_.connect(std::move(callback));
}
}