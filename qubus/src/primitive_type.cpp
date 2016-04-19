#include <qbb/qubus/primitive_object.hpp>

#include <utility>

using server_type = hpx::components::component<qbb::qubus::primitive_object>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_primitive_object);

namespace qbb
{
namespace qubus
{
primitive_object::primitive_object(type object_type_)
: object_type_(std::move(object_type_))
{
}

type primitive_object::object_type() const
{
    return object_type_;
}

hpx::id_type primitive_object::id() const
{
    return get_id();
}

void primitive_object::register_instance(local_address_space::handle data)
{
    data_ = std::move(data);
}

}
}
