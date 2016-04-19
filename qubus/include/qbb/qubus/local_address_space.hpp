#ifndef QBB_QUBUS_LOCAL_ADDRESS_SPACE_HPP
#define QBB_QUBUS_LOCAL_ADDRESS_SPACE_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/object.hpp>
#include <qbb/qubus/address.hpp>

#include <qbb/qubus/allocator.hpp>
#include <qbb/qubus/memory_block.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/util/integers.hpp>
#include <qbb/util/delegate.hpp>
#include <qbb/util/dense_hash_map.hpp>

#include <hpx/include/local_lcos.hpp>
#include <hpx/include/lcos.hpp>

#include <mutex>
#include <map>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>

namespace qbb
{
namespace qubus
{

inline address make_address_from_id(const hpx::id_type &id)
{
    return address(id.get_msb(), id.get_lsb());
}

class host_address_translation_table
{
public:
    host_address_translation_table();

    void register_mapping(address addr, void* ptr);

    void invalidate_mapping(address addr);

    void* resolve_address(address addr) const;

    const void* get_table() const;
    std::size_t bucket_count() const;
private:
    util::dense_hash_map<address, void*> translation_table_;
};

class address_space
{
private:
    class address_entry
    {
    public:
        address_entry(address addr_, std::unique_ptr<memory_block> data_);
        ~address_entry();

        address_entry(const address_entry&) = delete;
        address_entry& operator=(const address_entry&) = delete;

        address_entry(address_entry&& other);
        address_entry& operator=(address_entry&& other);

        const address& addr() const;
        memory_block& data() const;

        void pin();
        void unpin();

        bool is_pinned() const;

        void on_delete(std::function<void(address)> callback);
    private:
        address addr_;
        std::unique_ptr<memory_block> data_;
        std::atomic<int> pin_count_;
        util::delegate<void(address)> on_delete_;
    };

public:
    class pin
    {
    public:
        explicit pin(std::shared_ptr<address_entry> entry_);
        ~pin();

        pin(const pin&) = delete;
        pin& operator=(const pin&) = delete;

        pin(pin&& other) = default;
        pin& operator=(pin&& other) = default;

        address addr() const;

        memory_block& data() const;
    private:
        std::shared_ptr<address_entry> entry_;
    };

    class handle
    {
    public:
        handle() = default;
        explicit handle(std::shared_ptr<address_entry> entry_);

        handle(const handle&) = delete;
        handle& operator=(const handle&) = delete;

        handle(handle&& other) = default;
        handle& operator=(handle&& other) = default;

        address addr() const;

        explicit operator bool() const;

        hpx::future<pin> pin_object();
    private:
        std::shared_ptr<address_entry> entry_;
    };

    explicit address_space(std::unique_ptr<allocator> allocator_);

    handle allocate_object_page(const object_client& obj, long int size, long int alignment);
    hpx::future<handle> resolve_object(const object_client& obj);
    handle try_resolve_object(const object_client& obj) const;

    const void* get_address_translation_table() const;
    std::size_t bucket_count() const;

    void on_deallocation(std::function<void(address)> callback);
    void on_page_fault(std::function<hpx::future<handle>(address)> callback);

    void dump() const;
private:
    bool evict_objects(std::size_t hint);

    std::unique_ptr<allocator> allocator_;

    mutable host_address_translation_table translation_table_;
    mutable std::map<address, std::shared_ptr<address_entry>> entry_table_;
    mutable hpx::lcos::local::mutex address_translation_table_mutex_;

    util::delegate<void(address)> on_deallocation_;
    util::delegate<hpx::future<handle>(address)> on_page_fault_;
};

using host_address_space = address_space;

// TODO: Implement cache_address_space
// TODO: Implement archive_address_space

class local_address_space
{
public:
    using handle = address_space::handle;

    explicit local_address_space(host_address_space& host_addr_space_);

    handle allocate_object_page(const object_client& obj, long int size, long int alignment);
    hpx::future<handle> resolve_object(const object_client& obj);
    handle try_resolve_object(const object_client& obj) const;

private:
    host_address_space* host_addr_space_;
};

}
}

#endif