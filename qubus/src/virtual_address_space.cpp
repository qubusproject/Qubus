#include <qubus/virtual_address_space.hpp>

#include <hpx/runtime/serialization/serialize.hpp>
#include <hpx/runtime/serialization/unique_ptr.hpp>

HPX_REGISTER_COMPONENT(hpx::components::component<qubus::virtual_address_space_wrapper>,
                       qubus_virtual_address_space_wrapper);

using resolve_object_action = qubus::virtual_address_space_wrapper::resolve_object_action;
HPX_REGISTER_ACTION_DECLARATION(resolve_object_action,
                                qubus_virtual_address_space_wrapper_resolve_object_action);
HPX_REGISTER_ACTION(resolve_object_action,
                    qubus_virtual_address_space_wrapper_resolve_object_action);

using try_resolve_object_action = qubus::virtual_address_space_wrapper::try_resolve_object_action;
HPX_REGISTER_ACTION_DECLARATION(try_resolve_object_action,
                                qubus_virtual_address_space_wrapper_try_resolve_object_action);
HPX_REGISTER_ACTION(try_resolve_object_action,
                    qubus_virtual_address_space_wrapper_try_resolve_object_action);

namespace qubus
{

virtual_address_space_wrapper::client::client(hpx::id_type&& id) : base_type(std::move(id))
{
}

virtual_address_space_wrapper::client::client(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

hpx::future<object_instance> virtual_address_space_wrapper::client::resolve_object(const object& obj)
{
    return hpx::async<virtual_address_space_wrapper::resolve_object_action>(this->get_id(), obj);
}

object_instance virtual_address_space_wrapper::client::try_resolve_object(const object& obj) const
{
    return hpx::async<virtual_address_space_wrapper::try_resolve_object_action>(this->get_id(), obj)
        .get();
}

hpx::future<object_instance> virtual_address_space_wrapper::resolve_object(const object& obj)
{
    return wrapped_address_space_->resolve_object(obj);
}

object_instance virtual_address_space_wrapper::try_resolve_object(const object& obj) const
{
    return wrapped_address_space_->try_resolve_object(obj);
}
}