#include <qbb/qubus/local_object_factory.hpp>

#include <qbb/qubus/host_object_views.hpp>

#include <qbb/qubus/hpx_utils.hpp>

#include <qbb/util/assert.hpp>
#include <qbb/util/integers.hpp>
#include <qbb/util/unused.hpp>

#include <type_traits>
#include <utility>

using server_type = hpx::components::component<qubus::local_object_factory_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_local_object_factory_server);

using create_scalar_action = qubus::local_object_factory_server::create_scalar_action;
HPX_REGISTER_ACTION_DECLARATION(create_scalar_action,
                                qubus_local_object_factory_create_scalar_action);
HPX_REGISTER_ACTION(create_scalar_action, qubus_local_object_factory_create_scalar_action);

using create_array_action = qubus::local_object_factory_server::create_array_action;
HPX_REGISTER_ACTION_DECLARATION(create_array_action,
                                qubus_local_object_factory_create_array_action);
HPX_REGISTER_ACTION(create_array_action, qubus_local_object_factory_create_array_action);

using create_struct_action = qubus::local_object_factory_server::create_struct_action;
HPX_REGISTER_ACTION_DECLARATION(create_struct_action,
                                qubus_local_object_factory_create_struct_action);
HPX_REGISTER_ACTION(create_struct_action, qubus_local_object_factory_create_struct_action);

namespace qubus
{

local_object_factory_server::local_object_factory_server(local_address_space* address_space_)
: address_space_(address_space_)
{
}

hpx::future<hpx::id_type> local_object_factory_server::create_scalar(type data_type)
{
    QBB_ASSERT(data_type.is_primitive(), "The type of scalars has to be primitive.");

    object obj = new_here<object_server>(data_type);

    auto size = abi_.get_size_of(data_type);
    auto alignment = abi_.get_align_of(data_type);

    auto instance = address_space_->allocate_object_page(obj, size, alignment);

    auto obj_ptr = hpx::get_ptr<object_server>(hpx::launch::sync, obj.get_id());

    obj_ptr->register_instance(std::move(instance));

    return hpx::make_ready_future(obj.get());
}

hpx::future<hpx::id_type>
local_object_factory_server::create_array(type value_type, std::vector<util::index_t> shape)
{
    object obj = new_here<object_server>(types::array(value_type, util::to_uindex(shape.size())));

    auto layout = abi_.get_array_layout(value_type, shape);

    auto instance = address_space_->allocate_object_page(obj, util::to_uindex(layout.size()),
                                                         util::to_uindex(layout.alignment()));

    auto& mem_block = instance.data();

    void* data = mem_block.ptr();

    array_metadata* metadata = static_cast<array_metadata*>(data);

    metadata->shape = static_cast<char*>(data) + layout.shape_offset();
    metadata->data = static_cast<char*>(data) + layout.data_offset();

    util::index_t* current_shape = static_cast<util::index_t*>(metadata->shape);

    for (std::size_t i = 0; i < shape.size(); ++i)
    {
        current_shape[i] = shape[i];
    }

    auto obj_ptr = hpx::get_ptr<object_server>(hpx::launch::sync, obj.get_id());

    obj_ptr->register_instance(std::move(instance));

    return hpx::make_ready_future(obj.get());
}

hpx::future<hpx::id_type> local_object_factory_server::create_struct(type struct_type,
                                                                     std::vector<object> members)
{
    object obj = new_here<object_server>(struct_type);

    auto obj_ptr = hpx::get_ptr<object_server>(hpx::launch::sync, obj.get_id());

    for (const auto& member : members)
    {
        obj_ptr->add_component(member);
    }

    return hpx::make_ready_future(obj.get());
}

local_object_factory::local_object_factory(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

object local_object_factory::create_scalar(type data_type)
{
    return hpx::async<local_object_factory_server::create_scalar_action>(this->get_id(),
                                                                         std::move(data_type));
}

object local_object_factory::create_array(type value_type, std::vector<util::index_t> shape)
{
    return hpx::async<local_object_factory_server::create_array_action>(
        this->get_id(), std::move(value_type), std::move(shape));
}

object local_object_factory::create_struct(type struct_type, std::vector<object> members)
{
    return hpx::async<local_object_factory_server::create_struct_action>(
        this->get_id(), std::move(struct_type), std::move(members));
}
}