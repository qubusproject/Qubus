#include <qbb/qubus/local_address_space.hpp>

#include <qbb/qubus/object.hpp>

#include <qbb/qubus/evicting_allocator.hpp>

#include <qbb/qubus/logging.hpp>

#include <qbb/util/make_unique.hpp>
#include <qbb/util/unused.hpp>
#include <qbb/util/assert.hpp>

#include <utility>
#include <tuple>
#include <exception>

namespace qbb
{
namespace qubus
{

host_address_translation_table::host_address_translation_table() : translation_table_(nulladdr)
{
}

void host_address_translation_table::register_mapping(address addr, void* ptr)
{
    // TODO: Add error handling
    translation_table_.emplace(addr, ptr);
}

void host_address_translation_table::invalidate_mapping(address addr)
{
    translation_table_.erase(addr);
}

void* host_address_translation_table::resolve_address(address addr) const
{
    return translation_table_.at(addr);
}

const void* host_address_translation_table::get_table() const
{
    return translation_table_.data();
}

std::size_t host_address_translation_table::bucket_count() const
{
    return translation_table_.bucket_count();
}

address_space::handle::handle(std::shared_ptr<address_entry> entry_) : entry_(std::move(entry_))
{
}

address address_space::handle::addr() const
{
    return entry_->addr();
}

address_space::handle::operator bool() const
{
    return static_cast<bool>(entry_);
}

hpx::future<address_space::pin> address_space::handle::pin_object()
{
    entry_->pin();

    return hpx::make_ready_future(pin(entry_));
}

address_space::pin::pin(std::shared_ptr<address_entry> entry_) : entry_(std::move(entry_))
{
}

address_space::pin::~pin()
{
    if (entry_)
    {
        entry_->unpin();
    }
}

address address_space::pin::addr() const
{
    return entry_->addr();
}

memory_block& address_space::pin::data() const
{
    return entry_->data();
}

address_space::address_entry::address_entry(address addr_, std::unique_ptr<memory_block> data_)
: addr_(std::move(addr_)), data_(std::move(data_)), pin_count_(0), on_delete_([](address)
                                                                              {
                                                                              })
{
}

address_space::address_entry::~address_entry()
{
    on_delete_(addr_);
}

address_space::address_entry::address_entry(address_space::address_entry&& other)
: addr_(std::move(other.addr_)), data_(std::move(other.data_)), pin_count_(other.pin_count_.load()),
  on_delete_(std::move(other.on_delete_))
{
}

address_space::address_entry& address_space::address_entry::
operator=(address_space::address_entry&& other)
{
    addr_ = std::move(other.addr_);
    data_ = std::move(other.data_);
    pin_count_.store(other.pin_count_.load());
    on_delete_ = std::move(other.on_delete_);

    return *this;
}

const address& address_space::address_entry::addr() const
{
    return addr_;
}

memory_block& address_space::address_entry::data() const
{
    return *data_;
}

void address_space::address_entry::pin()
{
    pin_count_.fetch_add(1);
}

void address_space::address_entry::unpin()
{
    pin_count_.fetch_sub(1);
}

bool address_space::address_entry::is_pinned() const
{
    return pin_count_.load() > 0;
}

void address_space::address_entry::on_delete(std::function<void(address)> callback)
{
    on_delete_.connect(callback);
}

address_space::address_space(std::unique_ptr<allocator> allocator_)
: allocator_(std::make_unique<evicting_allocator>(std::move(allocator_),
                                                  [this](std::size_t hint)
                                                  {
                                                      return evict_objects(hint);
                                                  })),
  on_deallocation_([](address)
                   {
                   }),
  on_page_fault_([](address)
                 {
                     // TODO: Add correct exception type.
                     return hpx::make_exceptional_future<address_space::handle>(
                         std::runtime_error("Page fault"));
                 })
{
}

address_space::handle address_space::allocate_object_page(const object& obj, long int size,
                                                          long int alignment)
{
    std::unique_lock<hpx::lcos::local::mutex> guard(address_translation_table_mutex_);

    auto addr = make_address_from_id(obj.id());

    auto data = allocator_->allocate(size, alignment);

    translation_table_.register_mapping(addr, data->ptr());

    bool addr_was_free;
    using entry_table_iterator = decltype(entry_table_.begin());
    entry_table_iterator pos;

    std::tie(pos, addr_was_free) =
        entry_table_.emplace(addr, std::make_shared<address_entry>(addr, std::move(data)));

    QBB_ASSERT(addr_was_free, "Address is spuriously occupied.");

    pos->second->on_delete([this](address addr)
                           {
                               translation_table_.invalidate_mapping(addr);
                           });

    return address_space::handle(pos->second);
}

hpx::future<address_space::handle> address_space::resolve_object(const object& obj)
{
    std::unique_lock<hpx::lcos::local::mutex> guard(address_translation_table_mutex_);

    auto addr = make_address_from_id(obj.id());

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
            return on_page_fault_(addr);
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

    auto addr = make_address_from_id(obj.id());

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

const void* address_space::get_address_translation_table() const
{
    return translation_table_.get_table();
}

std::size_t address_space::bucket_count() const
{
    return translation_table_.bucket_count();
}

void address_space::on_deallocation(std::function<void(address)> callback)
{
    on_deallocation_.connect(std::move(callback));
}

void address_space::on_page_fault(
    std::function<hpx::future<address_space::handle>(address)> callback)
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
: host_addr_space_(&host_addr_space_)
{
}

local_address_space::handle
local_address_space::allocate_object_page(const object& obj, long int size, long int alignment)
{
    return host_addr_space_->allocate_object_page(obj, size, alignment);
}

hpx::future<local_address_space::handle> local_address_space::resolve_object(const object& obj)
{
    return host_addr_space_->resolve_object(obj);
}

local_address_space::handle local_address_space::try_resolve_object(const object& obj) const
{
    return host_addr_space_->try_resolve_object(obj);
}
}
}