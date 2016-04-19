#include <qbb/qubus/local_object_factory.hpp>

#include <qbb/qubus/primitive_object.hpp>

#include <qbb/qubus/host_objects_views.hpp>

#include <qbb/qubus/hpx_utils.hpp>

#include <qbb/util/integers.hpp>

#include <utility>

using server_type = hpx::components::component<qbb::qubus::local_object_factory_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_local_object_factory_server);

using create_array_action = qbb::qubus::local_object_factory_server::create_array_action;
HPX_REGISTER_ACTION_DECLARATION(create_array_action);
HPX_REGISTER_ACTION(create_array_action);

namespace qbb
{
namespace qubus
{

local_object_factory_server::local_object_factory_server(local_address_space* address_space_)
: address_space_(address_space_)
{
}

object_client local_object_factory_server::create_array(type value_type,
                                                        std::vector<util::index_t> shape)
{
    object_client obj = new_here<primitive_object>(types::array(value_type));

    auto layout = abi_.get_array_layout(value_type, shape);

    auto instance = address_space_->allocate_object_page(obj, util::to_uindex(layout.size()), util::to_uindex(layout.alignment()));

    auto pin = instance.pin_object().get();

    auto& mem_block = pin.data();

    void* data = mem_block.ptr();

    array_metadata* metadata = static_cast<array_metadata*>(data);

    metadata->shape = static_cast<char*>(data) + layout.shape_offset();
    metadata->data = static_cast<char*>(data) + layout.data_offset();

    util::index_t* current_shape = static_cast<util::index_t*>(metadata->shape);

    for (std::size_t i = 0; i < shape.size(); ++i)
    {
        current_shape[i] = shape[i];
    }

    auto obj_ptr = hpx::get_ptr_sync<primitive_object>(obj.get_id());

    obj_ptr->register_instance(std::move(instance));

    return obj;
}

local_object_factory::local_object_factory(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

object_client local_object_factory::create_array(type value_type, std::vector<util::index_t> shape)
{
    return hpx::async<local_object_factory_server::create_array_action>(
               this->get_id(), std::move(value_type), std::move(shape))
        .get();
}
}
}