#include <qbb/qubus/object.hpp>

#include <qbb/qubus/local_runtime.hpp>

#include <utility>

using server_type = hpx::components::component<qubus::object_server>;
HPX_REGISTER_COMPONENT(server_type, QUBUS_object_server);

using object_type_action = qubus::object_server::object_type_action;
HPX_REGISTER_ACTION_DECLARATION(object_type_action, qubus_object_server_type_action);
HPX_REGISTER_ACTION(object_type_action, qubus_object_server_type_action);

using acquire_read_access_action = qubus::object_server::acquire_read_access_action;
HPX_REGISTER_ACTION_DECLARATION(acquire_read_access_action, qubus_object_server_acquire_read_access_action);
HPX_REGISTER_ACTION(acquire_read_access_action, qubus_object_server_acquire_read_access_action);

using acquire_write_access_action = qubus::object_server::acquire_write_access_action;
HPX_REGISTER_ACTION_DECLARATION(acquire_write_access_action, qubus_object_server_acquire_write_access_action);
HPX_REGISTER_ACTION(acquire_write_access_action, qubus_object_server_acquire_write_access_action);

using components_action = qubus::object_server::components_action;
HPX_REGISTER_ACTION_DECLARATION(components_action, qubus_object_server_components_action);
HPX_REGISTER_ACTION(components_action, qubus_object_server_components_action);

namespace qubus
{

object_server::object_server(type object_type_)
: object_type_(std::move(object_type_))
{
}

type object_server::object_type() const
{
    return object_type_;
}

object_server::~object_server()
{
    get_local_runtime().get_address_space().free_object(object(id()));
}

hpx::id_type object_server::id() const
{
    return get_id();
}

token object_server::acquire_read_access()
{
    return monitor_.acquire_read_access().get();
}

token object_server::acquire_write_access()
{
    return monitor_.acquire_write_access().get();
}

std::vector<object> object_server::components() const
{
    return components_;
}

void object_server::register_instance(local_address_space::handle data)
{
    data_ = std::move(data);
}

void object_server::add_component(const object& component)
{
    components_.push_back(component);
}

const object& object_server::operator()(long int index) const
{
    return components_.at(index);
}

object::object(hpx::id_type&& id) : base_type(std::move(id))
{
}

object::object(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

type object::object_type() const
{
    return hpx::async<object_server::object_type_action>(this->get_id()).get();
}

hpx::id_type object::id() const
{
    return get_id();
}

hpx::future<token> object::acquire_read_access()
{
    return hpx::async<object_server::acquire_read_access_action>(this->get_id()).then([] (hpx::future<hpx::id_type> id) { return token(std::move(id)); });
}

hpx::future<token> object::acquire_write_access()
{
    return hpx::async<object_server::acquire_write_access_action>(this->get_id()).then([] (hpx::future<hpx::id_type> id) { return token(std::move(id)); });
}

std::vector<object> object::components() const
{
    return hpx::async<object_server::components_action>(this->get_id()).get();
}
}