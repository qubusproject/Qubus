#ifndef QUBUS_LOCAL_ADDRESS_SPACE_HPP
#define QUBUS_LOCAL_ADDRESS_SPACE_HPP

#include <hpx/config.hpp>

#include <qubus/allocator.hpp>
#include <qubus/host_memory.hpp>
#include <qubus/instance_handle.hpp>
#include <qubus/logging.hpp>
#include <qubus/object_id.hpp>
#include <qubus/object_instance.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/delegate.hpp>
#include <qubus/util/dense_hash_map.hpp>
#include <qubus/util/handle.hpp>
#include <qubus/util/integers.hpp>
#include <qubus/util/unused.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/local_lcos.hpp>
#include <hpx/parallel/executors.hpp> // Workaround for missing includes.
#include <hpx/runtime/threads/executors/pool_executor.hpp>

#include <exception>
#include <functional>
#include <map>
#include <mutex>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace qubus
{

template <typename MemoryType>
class address_space
{
public:
    using object_instance_type = object_instance<MemoryType>;
    using handle = instance_handle<MemoryType>;

    class page_fault_context
    {
    public:
        explicit page_fault_context(address_space& addr_space_) : addr_space_(&addr_space_)
        {
        }

        hpx::future<object_instance<MemoryType>> allocate_raw_page(type datatype, long int size)
        {
            QUBUS_ASSERT(addr_space_, "Invalid context.");

            return addr_space_->allocate_raw_page(std::move(datatype), size);
        }

    private:
        address_space* addr_space_;
    };

    address_space(MemoryType allocator_, hpx::threads::executors::pool_executor& service_executor_)
    : allocator_(std::move(allocator_)),
      on_page_fault_([](object_id, page_fault_context) {
          // TODO: Add correct exception type.
          return hpx::make_exceptional_future<address_space::object_instance_type>(
              std::runtime_error("Page fault"));
      }),
      service_executor_(&service_executor_)
    {
    }

    template <typename Constructor>
    hpx::shared_future<handle> allocate_page(object_id id, type datatype, long int size,
                                             Constructor constructor)
    {
        auto data = [&] {
            if constexpr (allocator_traits<MemoryType>::has_alignment_support)
            {
                return allocator_.allocate(size, 1);
            }
            else
            {
                return allocator_.allocate(size);
            }
        }();

        auto instance =
            std::make_shared<object_instance<MemoryType>>(std::move(datatype), std::move(data));

        constructor(*instance).get();

        handle h(std::move(instance));

        bool addr_was_free;
        using entry_table_iterator = decltype(entry_table_.begin());
        entry_table_iterator pos;

        {
            std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

            std::tie(pos, addr_was_free) = entry_table_.emplace(id, hpx::make_ready_future(h));
        }

        QUBUS_ASSERT(addr_was_free, "Address is spuriously occupied.");

        return pos->second;
    }

    void register_page(object_id id, handle page)
    {
        bool addr_was_free;
        using entry_table_iterator = decltype(entry_table_.begin());
        entry_table_iterator pos;

        {
            std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

            std::tie(pos, addr_was_free) =
                entry_table_.emplace(id, hpx::make_ready_future(std::move(page)));
        }

        QUBUS_ASSERT(addr_was_free, "Address is spuriously occupied.");
    }

    void free_object(object_id id)
    {
        std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

        entry_table_.erase(id);
    }

    hpx::shared_future<handle> resolve_object(object_id id)
    {
        std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

        auto entry = entry_table_.find(id);

        if (entry != entry_table_.end())
            return entry->second;

        try
        {
            hpx::future<object_instance_type> instance = on_page_fault_(id, page_fault_context(*this));

            handle h(std::make_shared<object_instance_type>(instance.get()));

            auto [pos, addr_was_free] = entry_table_.emplace(id, hpx::make_ready_future(std::move(h)));

            QUBUS_ASSERT(addr_was_free, "Address is spuriously occupied.");

            return pos->second;
        }
        catch (...)
        {
            throw 0;
        }
    }

    handle try_resolve_object(object_id id) const
    {
        std::unique_lock<hpx::lcos::local::spinlock> guard(address_translation_table_mutex_);

        auto entry = entry_table_.find(id);

        if (entry != entry_table_.end())
        {
            if (entry->second.is_ready())
            {
                return handle(entry->second.get());
            }
        }

        return handle();
    }

    void on_page_fault(std::function<hpx::future<object_instance_type>(object_id, page_fault_context)> callback)
    {
        on_page_fault_.connect(std::move(callback));
    }

    void dump() const
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

private:
    hpx::future<object_instance_type> allocate_raw_page(type datatype, long int size)
    {
        auto data = [&] {
            if constexpr (allocator_traits<MemoryType>::has_alignment_support)
            {
                return allocator_.allocate(size, 1);
            }
            else
            {
                return allocator_.allocate(size);
            }
        }();

        return hpx::make_ready_future(object_instance<MemoryType>(std::move(datatype), std::move(data)));
    }

    bool evict_objects(std::size_t hint)
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

    MemoryType allocator_;

    mutable util::dense_hash_map<object_id, hpx::shared_future<handle>> entry_table_;
    mutable hpx::lcos::local::spinlock address_translation_table_mutex_;

    util::delegate<hpx::future<object_instance_type>(object_id, page_fault_context)> on_page_fault_;

    hpx::threads::executors::pool_executor* service_executor_;
};

using host_address_space = address_space<host_allocator>;

// TODO: Implement cache_address_space
// TODO: Implement archive_address_space

class local_address_space
{
public:
    using object_instance_type = host_address_space::object_instance_type;
    using handle = host_address_space::handle;

    class page_fault_context
    {
    public:
        explicit page_fault_context(host_address_space::page_fault_context host_ctx_);

        //handle allocate_page(long int size, long int alignment);
    private:
        host_address_space::page_fault_context host_ctx_;
    };

    explicit local_address_space(host_address_space& host_addr_space_);

    //handle allocate_page(long int size, long int alignment);
    void register_page(object_id obj, handle page);

    void free_object(object_id obj);
    hpx::shared_future<handle> resolve_object(object_id obj);
    handle try_resolve_object(object_id obj) const;

    void on_page_fault(std::function<hpx::future<object_instance_type>(object_id, page_fault_context)> callback);

    host_address_space& host_addr_space();

private:
    std::reference_wrapper<host_address_space> host_addr_space_;

    util::delegate<hpx::future<object_instance_type>(object_id, page_fault_context)> on_page_fault_;
};
} // namespace qubus

#endif