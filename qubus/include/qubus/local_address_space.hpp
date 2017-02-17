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

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include <atomic>
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
private:
    class address_entry
    {
    public:
        address_entry(hpx::naming::gid_type addr_, std::unique_ptr<memory_block> data_);

        address_entry(const address_entry&) = delete;
        address_entry& operator=(const address_entry&) = delete;

        address_entry(address_entry&& other) = default;
        address_entry& operator=(address_entry&& other) = default;

        const hpx::naming::gid_type& addr() const;
        memory_block& data() const;

    private:
        hpx::naming::gid_type addr_;
        std::unique_ptr<memory_block> data_;
    };

public:
    class handle
    {
    public:
        handle() = default;
        explicit handle(std::shared_ptr<address_entry> entry_);

        handle(const handle&) = delete;
        handle& operator=(const handle&) = delete;

        handle(handle&& other) = default;
        handle& operator=(handle&& other) = default;

        memory_block& data() const;

        explicit operator bool() const;

    private:
        std::shared_ptr<address_entry> entry_;
    };

    explicit address_space(std::unique_ptr<allocator> allocator_);

    handle allocate_object_page(const object& obj, long int size, long int alignment);
    void free_object(const object& obj);
    hpx::future<handle> resolve_object(const object& obj);
    handle try_resolve_object(const object& obj) const;

    void on_page_fault(std::function<hpx::future<handle>(const object& obj)> callback);

    void dump() const;

private:
    bool evict_objects(std::size_t hint);

    std::unique_ptr<allocator> allocator_;

    mutable std::map<hpx::naming::gid_type, std::shared_ptr<address_entry>> entry_table_;
    mutable hpx::lcos::local::mutex address_translation_table_mutex_;

    util::delegate<hpx::future<handle>(const object& obj)> on_page_fault_;
};

using host_address_space = address_space;

// TODO: Implement cache_address_space
// TODO: Implement archive_address_space

class local_address_space
{
public:
    using handle = host_address_space::handle;

    explicit local_address_space(host_address_space& host_addr_space_);

    handle allocate_object_page(const object& obj, long int size, long int alignment);
    void free_object(const object& obj);
    hpx::future<handle> resolve_object(const object& obj);
    handle try_resolve_object(const object& obj) const;

private:
    std::reference_wrapper<host_address_space> host_addr_space_;
};
}

#endif