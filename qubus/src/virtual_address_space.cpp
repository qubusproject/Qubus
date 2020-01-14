#include <qubus/virtual_address_space.hpp>

#include <qubus/util/hpx/serialization/optional.hpp>

#include <hpx/runtime/serialization/serialize.hpp>
#include <hpx/runtime/serialization/unique_ptr.hpp>

#include <qubus/util/unused.hpp>

HPX_REGISTER_COMPONENT(hpx::components::component<qubus::virtual_address_space_wrapper>,
                       qubus_virtual_address_space_wrapper);

using resolve_object_action = qubus::virtual_address_space_wrapper::resolve_object_action;
HPX_REGISTER_ACTION(resolve_object_action,
                    qubus_virtual_address_space_wrapper_resolve_object_action);

using invalidate_object_action = qubus::virtual_address_space_wrapper::invalidate_object_action;
HPX_REGISTER_ACTION(invalidate_object_action,
                    qubus_virtual_address_space_wrapper_invalidate_object_action);

using free_object_action = qubus::virtual_address_space_wrapper::free_object_action;
HPX_REGISTER_ACTION(free_object_action,
                    qubus_virtual_address_space_wrapper_free_object_action);

namespace qubus
{

virtual_address_space_wrapper::client::client(hpx::id_type&& id) : base_type(std::move(id))
{
}

virtual_address_space_wrapper::client::client(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

hpx::future<instance_token> virtual_address_space_wrapper::client::resolve_object(object_id id)
{
    return hpx::async<virtual_address_space_wrapper::resolve_object_action>(this->get_id(), id);
}

hpx::future<void> virtual_address_space_wrapper::client::invalidate_object(object_id id)
{
    return hpx::async<virtual_address_space_wrapper::invalidate_object_action>(this->get_id(), id);
}

hpx::future<void> virtual_address_space_wrapper::client::free_object(object_id id)
{
    return hpx::async<virtual_address_space_wrapper::free_object_action>(this->get_id(), id);
}

virtual_address_space_wrapper::virtual_address_space_wrapper(
    std::unique_ptr<virtual_address_space> wrapped_address_space_)
: wrapped_address_space_(std::move(wrapped_address_space_))
{
}

hpx::future<instance_token> virtual_address_space_wrapper::resolve_object(object_id id)
{
    return wrapped_address_space_->resolve_object(id);
}

hpx::future<void> virtual_address_space_wrapper::invalidate_object(object_id id)
{
    return wrapped_address_space_->invalidate_object(id);
}

hpx::future<void> virtual_address_space_wrapper::free_object(object_id id)
{
    return wrapped_address_space_->free_object(id);
}

} // namespace qubus