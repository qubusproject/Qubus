#ifndef QUBUS_LOCAL_ADDRESS_SPACE_HPP
#define QUBUS_LOCAL_ADDRESS_SPACE_HPP

#include <hpx/config.hpp>

#include <qubus/allocator.hpp>
#include <qubus/memory_block.hpp>

#include <qubus/util/delegate.hpp>
#include <qubus/util/dense_hash_map.hpp>
#include <qubus/util/handle.hpp>
#include <qubus/util/integers.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/local_lcos.hpp>

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace qubus
{

class object;

class address_space
{
public:
    class address_entry;

    class handle
    {
    public:
        handle() = default;
        explicit handle(std::shared_ptr<address_entry> entry_);

        memory_block& data() const;

        explicit operator bool() const;

        bool unique() const;

        void free();
    private:
        std::shared_ptr<address_entry> entry_;
    };

    class address_entry
    {
    public:
        explicit address_entry(std::unique_ptr<memory_block> data_);

        address_entry(const address_entry&) = delete;
        address_entry& operator=(const address_entry&) = delete;

        address_entry(address_entry&& other) = default;
        address_entry& operator=(address_entry&& other) = default;

        memory_block& data() const;

    private:
        std::unique_ptr<memory_block> data_;
    };

    class page_fault_context
    {
    public:
        explicit page_fault_context(address_space& addr_space_);

        handle allocate_page(long int size, long int alignment);

    private:
        address_space* addr_space_;
    };

    explicit address_space(std::unique_ptr<allocator> allocator_);

    handle allocate_page(long int size, long int alignment);
    void register_page(const object& obj, handle page);

    void free_object(const object& obj);
    hpx::future<handle> resolve_object(const object& obj);
    handle try_resolve_object(const object& obj) const;

    void on_page_fault(
        std::function<hpx::future<handle>(const object& obj, page_fault_context)> callback);

    void dump() const;

private:
    bool evict_objects(std::size_t hint);

    std::unique_ptr<allocator> allocator_;

    mutable util::dense_hash_map<hpx::naming::gid_type, hpx::shared_future<handle>> entry_table_;
    mutable hpx::lcos::local::spinlock address_translation_table_mutex_;

    util::delegate<hpx::future<handle>(const object&, page_fault_context)> on_page_fault_;
};

using host_address_space = address_space;

// TODO: Implement cache_address_space
// TODO: Implement archive_address_space

class local_address_space
{
public:
    using handle = host_address_space::handle;
    using address_entry = host_address_space::address_entry;

    class page_fault_context
    {
    public:
        explicit page_fault_context(host_address_space::page_fault_context host_ctx_);

        handle allocate_page(long int size, long int alignment);

    private:
        host_address_space::page_fault_context host_ctx_;
    };

    explicit local_address_space(host_address_space& host_addr_space_);

    handle allocate_page(long int size, long int alignment);
    void register_page(const object& obj, handle page);

    void free_object(const object& obj);
    hpx::future<handle> resolve_object(const object& obj);
    handle try_resolve_object(const object& obj) const;

    void on_page_fault(
        std::function<hpx::future<handle>(const object&, page_fault_context)> callback);

private:
    std::reference_wrapper<host_address_space> host_addr_space_;

    util::delegate<hpx::future<handle>(const object&, page_fault_context)> on_page_fault_;
};
}

#endif