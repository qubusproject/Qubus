#include <qubus/object.hpp>

#include <qubus/local_runtime.hpp>

#include <utility>

using server_type = hpx::components::component<qubus::object_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_object_server);

using object_type_action = qubus::object_server::object_type_action;
HPX_REGISTER_ACTION(object_type_action, qubus_object_server_type_action);

using size_action = qubus::object_server::size_action;
HPX_REGISTER_ACTION(size_action, qubus_object_server_size_action);

using alignment_action = qubus::object_server::alignment_action;
HPX_REGISTER_ACTION(alignment_action, qubus_object_server_alignment_action);

using primary_instance_action = qubus::object_server::primary_instance_action;
HPX_REGISTER_ACTION(primary_instance_action, qubus_object_server_primary_instance_action);

using has_data_action = qubus::object_server::has_data_action;
HPX_REGISTER_ACTION(has_data_action, qubus_object_server_has_data_action);

using acquire_read_access_action = qubus::object_server::acquire_read_access_action;
HPX_REGISTER_ACTION(acquire_read_access_action, qubus_object_server_acquire_read_access_action);

using acquire_write_access_action = qubus::object_server::acquire_write_access_action;
HPX_REGISTER_ACTION(acquire_write_access_action, qubus_object_server_acquire_write_access_action);

using components_action = qubus::object_server::components_action;
HPX_REGISTER_ACTION(components_action, qubus_object_server_components_action);

namespace qubus
{

object_server::object_server(type object_type_, std::size_t size_, std::size_t alignment_)
: object_type_(std::move(object_type_)), size_(size_), alignment_(alignment_)
{
}

type object_server::object_type() const
{
    return object_type_;
}

std::size_t object_server::size() const
{
    return size_;
}

std::size_t object_server::alignment() const
{
    return alignment_;
}

object_server::~object_server()
{
    //TODO: Send a notification to all parts of the memory subsystem that this object is dead.
}

hpx::id_type object_server::id() const
{
    return get_id();
}

object_instance object_server::primary_instance() const
{
    if (!has_data())
        return object_instance();

    return object_instance(data_);
}

bool object_server::has_data() const
{
    return static_cast<bool>(data_);
}

token object_server::acquire_read_access()
{
    return monitor_.acquire_read_access();
}

token object_server::acquire_write_access()
{
    return monitor_.acquire_write_access();
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

object::object(hpx::id_type id) : base_type(std::move(id))
{
}

object::object(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

type object::object_type() const
{
    return hpx::async<object_server::object_type_action>(this->get_id()).get();
}

std::size_t object::size() const
{
    return hpx::async<object_server::size_action>(this->get_id()).get();
}

std::size_t object::alignment() const
{
    return hpx::async<object_server::alignment_action>(this->get_id()).get();
}

hpx::id_type object::id() const
{
    return get_id();
}

object_instance object::primary_instance() const
{
    return hpx::async<object_server::primary_instance_action>(this->get_id()).get();
}

bool object::has_data() const
{
    return hpx::async<object_server::has_data_action>(this->get_id()).get();
}

token object::acquire_read_access()
{
    return hpx::async<object_server::acquire_read_access_action>(this->get_id()).get();
}

token object::acquire_write_access()
{
    return hpx::async<object_server::acquire_write_access_action>(this->get_id()).get();
}

std::vector<object> object::components() const
{
    return hpx::async<object_server::components_action>(this->get_id()).get();
}
}