#include <qubus/object_instance.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>

#include <qubus/hpx_utils.hpp>
#include <qubus/util/unused.hpp>

#include <cstdint>

namespace qubus
{
namespace
{
template <typename T>
class pointer_allocator
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    pointer_allocator() noexcept : pointer_(nullptr), size_(0)
    {
    }

    pointer_allocator(pointer pointer_, size_type size_) noexcept : pointer_(pointer_), size_(size_)
    {
    }

    pointer address(reference value) const
    {
        return &value;
    }

    const_pointer address(const_reference value) const
    {
        return &value;
    }

    pointer allocate(size_type size, void const* hint = nullptr)
    {
        QUBUS_ASSERT(size == size_, "'size' need to match the size of the wrapped buffer.");
        return static_cast<T*>(pointer_);
    }

    void deallocate(pointer p, size_type size)
    {
        QUBUS_ASSERT(p == pointer_ && size == size_, "The deallocated memory needs to be identical to the buffer.");
    }

private:
    friend class hpx::serialization::access;

    template <typename Archive>
    void load(Archive& ar, unsigned int QUBUS_UNUSED(version))
    {
        std::size_t t = 0;
        ar >> size_ >> t;
        pointer_ = reinterpret_cast<pointer>(t);
    }

    template <typename Archive>
    void save(Archive& ar, unsigned int QUBUS_UNUSEDversion) const
    {
        std::size_t t = reinterpret_cast<std::size_t>(pointer_);
        ar << size_ << t;
    }

    HPX_SERIALIZATION_SPLIT_MEMBER()

private:
    pointer pointer_;
    size_type size_;
};

using transfer_buffer_type = hpx::serialization::serialize_buffer<char, pointer_allocator<char>>;
}

class object_instance_server : public hpx::components::component_base<object_instance_server>
{
public:
    object_instance_server() = default;

    explicit object_instance_server(local_address_space::handle local_handle_);

    transfer_buffer_type copy(std::uintptr_t remote_addr) const;

    HPX_DEFINE_COMPONENT_ACTION(object_instance_server, copy, copy_action);

private:
    local_address_space::handle local_handle_;
};

class object_instance::impl
    : public hpx::components::client_base<object_instance::impl, object_instance_server>
{
public:
    using base_type = hpx::components::client_base<object_instance::impl, object_instance_server>;

    impl() = default;

    impl(hpx::id_type&& id);

    impl(hpx::future<hpx::id_type>&& id);

    hpx::future<void> copy(util::span<char> buffer) const;

private:
    static void transfer_data(util::span<char> recv, hpx::future<transfer_buffer_type> f)
    {
        transfer_buffer_type buffer(f.get());

        // If the output buffer and the input buffer are different, this was a local copy,
        // no data transfer has happened yet and we need to copy the data manually.
        if (buffer.data() != recv.data())
        {
            std::copy(buffer.data(), buffer.data() + buffer.size(), recv.begin());
        }
    }
};

object_instance::object_instance() : impl_(std::make_unique<impl>())
{
}

object_instance::object_instance(local_address_space::handle local_handle_)
: impl_(std::make_unique<impl>(new_here<object_instance_server>(std::move(local_handle_))))
{
}

object_instance::~object_instance() = default;

object_instance::object_instance(object_instance&& other) noexcept = default;
object_instance& object_instance::operator=(object_instance&& other) noexcept = default;

hpx::future<void> object_instance::copy(util::span<char> buffer) const
{
    return impl_->copy(buffer);
}

void object_instance::load(hpx::serialization::input_archive& ar, unsigned QUBUS_UNUSED(version))
{
    ar >> impl_;
}

void object_instance::save(hpx::serialization::output_archive& ar,
                           unsigned QUBUS_UNUSED(version)) const
{
    ar << impl_;
}
}

HPX_REGISTER_COMPONENT(hpx::components::component<qubus::object_instance_server>,
                       qubus_object_instance_server);

using copy_action = qubus::object_instance_server::copy_action;
HPX_REGISTER_ACTION_DECLARATION(copy_action, qubus_object_instance_server_copy_action);
HPX_REGISTER_ACTION(copy_action, qubus_object_instance_server_copy_action);

namespace qubus
{
object_instance_server::object_instance_server(local_address_space::handle local_handle_)
: local_handle_(std::move(local_handle_))
{
}

transfer_buffer_type object_instance_server::copy(std::uintptr_t remote_addr) const
{
    pointer_allocator<char> allocator(reinterpret_cast<char*>(remote_addr), 0);

    const auto& data = local_handle_.data();

    auto ptr = data.ptr();
    auto size = data.size();

    return transfer_buffer_type(static_cast<char*>(ptr), size, transfer_buffer_type::reference,
                                std::move(allocator));
}

object_instance::impl::impl(hpx::id_type&& id) : base_type(std::move(id))
{
}

object_instance::impl::impl(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

hpx::future<void> object_instance::impl::copy(util::span<char> buffer) const
{
    auto addr = reinterpret_cast<std::uintptr_t>(buffer.data());

    return hpx::async<object_instance_server::copy_action>(this->get_id(), addr)
        .then([buffer](hpx::future<transfer_buffer_type> buff) {
            transfer_data(buffer, std::move(buff));
        });
}
}