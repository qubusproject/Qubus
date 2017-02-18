#include <qubus/local_address_space.hpp>

#include <qubus/object.hpp>

#include <qubus/evicting_allocator.hpp>

#include <qubus/logging.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/unused.hpp>

#include <exception>
#include <tuple>
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

address_space::address_entry::address_entry(hpx::naming::gid_type addr_,
                                            std::unique_ptr<memory_block> data_)
: addr_(std::move(addr_)), data_(std::move(data_))
{
}

const hpx::naming::gid_type& address_space::address_entry::addr() const
{
    return addr_;
}

memory_block& address_space::address_entry::data() const
{
    return *data_;
}

address_space::address_space(std::unique_ptr<allocator> allocator_)
: allocator_(std::make_unique<evicting_allocator>(
      std::move(allocator_), [this](std::size_t hint) { return evict_objects(hint); })),
  on_page_fault_([](const object&) {
      // TODO: Add correct exception type.
      return hpx::make_exceptional_future<address_space::handle>(std::runtime_error("Page fault"));
  })
{
}

address_space::handle address_space::allocate_object_page(const object& obj, long int size,
                                                          long int alignment)
{
    std::unique_lock<hpx::lcos::local::mutex> guard(address_translation_table_mutex_);

    auto addr = obj.get_id().get_gid();

    auto data = allocator_->allocate(size, alignment);

    bool addr_was_free;
    using entry_table_iterator = decltype(entry_table_.begin());
    entry_table_iterator pos;

    std::tie(pos, addr_was_free) =
        entry_table_.emplace(addr, std::make_shared<address_entry>(addr, std::move(data)));

    QUBUS_ASSERT(addr_was_free, "Address is spuriously occupied.");

    return address_space::handle(pos->second);
}

void address_space::free_object(const object& obj)
{
    std::unique_lock<hpx::lcos::local::mutex> guard(address_translation_table_mutex_);

    entry_table_.erase(obj.get_id().get_gid());
}

hpx::future<address_space::handle> address_space::resolve_object(const object& obj)
{
    std::unique_lock<hpx::lcos::local::mutex> guard(address_translation_table_mutex_);

    auto addr = obj.get_id().get_gid();

    auto entry = entry_table_.find(addr);

    if (entry != entry_table_.end())
    {
        return hpx::make_ready_future(handle(entry->second));
    }
    else
    {
        // Unlock the mutex since on_page_fault_ might call other member functions.
        guard.unlock();

        try
        {
            return on_page_fault_(obj);
        }
        catch (...)
        {
            throw 0;
        }
    }
}

address_space::handle address_space::try_resolve_object(const object& obj) const
{
    std::unique_lock<hpx::lcos::local::mutex> guard(address_translation_table_mutex_);

    auto addr = obj.get_id().get_gid();

    auto entry = entry_table_.find(addr);

    if (entry != entry_table_.end())
    {
        return handle(entry->second);
    }
    else
    {
        return handle();
    }
}

bool address_space::evict_objects(std::size_t hint)
{
    std::unique_lock<hpx::lcos::local::mutex> guard(address_translation_table_mutex_);

    BOOST_LOG_NAMED_SCOPE("local_address_space");

    logger slg;

    std::size_t freed_memory = 0;

    for (const auto& entry : entry_table_)
    {
        if (entry.second.unique())
        {
            QUBUS_LOG(slg, info) << "evicting object " << entry.first;

            freed_memory += entry.second->data().size();

            entry_table_.erase(entry.first);

            if (freed_memory >= hint)
            {
                return true;
            }
        }
    }

    bool has_freed_memory = freed_memory > 0;

    return has_freed_memory;
}

void address_space::on_page_fault(
    std::function<hpx::future<address_space::handle>(const object& obj)> callback)
{
    on_page_fault_.connect(std::move(callback));
}

void address_space::dump() const
{
    std::unique_lock<hpx::lcos::local::mutex> guard(address_translation_table_mutex_);

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

local_address_space::local_address_space(host_address_space& host_addr_space_)
: host_addr_space_(host_addr_space_)
{
    host_addr_space_.on_page_fault([this](const object& obj) -> hpx::future<address_space::handle> {
        auto components = obj.components();

        if (!components.empty())
        {
            auto page = this->host_addr_space_.get().allocate_object_page(obj, sizeof(void*) * components.size(), sizeof(void*));

            auto ptr = static_cast<void**>(page.data().ptr());

            for (const auto& component : components)
            {
                auto component_page = this->host_addr_space_.get().resolve_object(component).get();

                *ptr = component_page.data().ptr();

                ++ptr;
            }

            return hpx::make_ready_future(std::move(page));
        }
        else
        {
            return hpx::make_exceptional_future<address_space::handle>(std::runtime_error("Page fault"));
        }
    });
}

local_address_space::handle
local_address_space::allocate_object_page(const object& obj, long int size, long int alignment)
{
    return host_addr_space_.get().allocate_object_page(obj, size, alignment);
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
}