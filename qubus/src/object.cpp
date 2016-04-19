#include <qbb/qubus/object.hpp>

#include <utility>

HPX_DEFINE_GET_COMPONENT_TYPE(qbb::qubus::object);

using object_type_action = qbb::qubus::object::object_type_action;
HPX_REGISTER_ACTION_DECLARATION(object_type_action);
HPX_REGISTER_ACTION(object_type_action);

using get_last_modification_action = qbb::qubus::object::get_last_modification_action;
HPX_REGISTER_ACTION_DECLARATION(get_last_modification_action);
HPX_REGISTER_ACTION(get_last_modification_action);

using record_modification_action = qbb::qubus::object::record_modification_action;
HPX_REGISTER_ACTION_DECLARATION(record_modification_action);
HPX_REGISTER_ACTION(record_modification_action);

namespace qbb
{
namespace qubus
{

basic_object::basic_object() : last_modification_(hpx::make_ready_future())
{
}

hpx::shared_future<void> basic_object::get_last_modification() const
{
    return last_modification_;
}

void basic_object::record_modification(hpx::shared_future<void> modification)
{
    last_modification_ = modification;
}

object_client::object_client(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

type object_client::object_type() const
{
    return hpx::async<object::object_type_action>(this->get_id()).get();
}

hpx::id_type object_client::id() const
{
    return get_id();
}

hpx::shared_future<void> object_client::get_last_modification() const
{
    return hpx::async<object::get_last_modification_action>(this->get_id());
}

void object_client::record_modification(hpx::shared_future<void> f)
{
    hpx::async<object::record_modification_action>(this->get_id(), std::move(f)).get();
}

}
}