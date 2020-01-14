#ifndef QUBUS_INSTANCE_TOKEN_HPP
#define QUBUS_INSTANCE_TOKEN_HPP

#include <qubus/host_memory.hpp>
#include <qubus/instance_handle.hpp>
#include <qubus/object_instance.hpp>

#include <qubus/util/bit.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <variant>

namespace qubus
{

class transferable_object_instance
{
public:
    transferable_object_instance() = default;

    transferable_object_instance(instance_handle<host_allocator> instance_,
                                 hpx::id_type source_locality_, std::uintptr_t allocator_addr_,
                                 std::size_t requested_alignment_);

    transferable_object_instance(const transferable_object_instance&) = delete;
    transferable_object_instance& operator=(const transferable_object_instance&) = delete;

    transferable_object_instance(transferable_object_instance&&) = default;
    transferable_object_instance& operator=(transferable_object_instance&&) = default;

    object_instance<host_allocator> unwrap();

    void load(hpx::serialization::input_archive& ar, unsigned int version);

    void save(hpx::serialization::output_archive& ar, unsigned int version) const;

    HPX_SERIALIZATION_SPLIT_MEMBER();

private:
    std::variant<std::monostate, object_instance<host_allocator>, instance_handle<host_allocator>> instance_;
    hpx::id_type source_locality_;
    std::uintptr_t allocator_addr_ = 0;
    std::size_t requested_alignment_ = 0;
};

class instance_token_server : public hpx::components::component_base<instance_token_server>
{
public:
    explicit instance_token_server(instance_handle<host_allocator> handle_);

    transferable_object_instance acquire_instance(hpx::id_type source_locality,
                                                  std::uintptr_t allocator_addr,
                                                  std::size_t requested_alignment) const;

    HPX_DEFINE_COMPONENT_ACTION(instance_token_server, acquire_instance, acquire_instance_action);

private:
    instance_handle<host_allocator> handle_;
};

class instance_token : public hpx::components::client_base<instance_token, instance_token_server>
{
public:
    using base_type = hpx::components::client_base<instance_token, instance_token_server>;

    instance_token() = default;

    instance_token(hpx::id_type&& id);
    instance_token(hpx::future<hpx::id_type>&& id);

    template <typename Executor>
    hpx::future<object_instance<host_allocator>>
    acquire_instance(Executor& executor, host_allocator& allocator, std::size_t alignment) const
    {
        auto allocator_addr = util::bit_cast<std::uintptr_t>(&allocator);

        auto transfered_instance = hpx::async<instance_token_server::acquire_instance_action>(
            this->get_id(), hpx::find_here(), allocator_addr, alignment);

        return transfered_instance.then(
            executor, [](hpx::future<transferable_object_instance> transfered_instance) {
                return transfered_instance.get().unwrap();
            });
    }
};

} // namespace qubus

#endif
