#include <qubus/local_object_factory.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/integers.hpp>
#include <qubus/util/unused.hpp>

#include <boost/range/adaptor/indexed.hpp>

#include <type_traits>
#include <utility>

using server_type = hpx::components::component<qubus::local_object_factory_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_local_object_factory_server);

using create_native_object_action = qubus::local_object_factory_server::create_native_object_action;
HPX_REGISTER_ACTION(create_native_object_action,
                    qubus_local_object_factory_create_native_object_action);

using create_scalar_action = qubus::local_object_factory_server::create_scalar_action;
HPX_REGISTER_ACTION(create_scalar_action, qubus_local_object_factory_create_scalar_action);

using create_array_action = qubus::local_object_factory_server::create_array_action;
HPX_REGISTER_ACTION(create_array_action, qubus_local_object_factory_create_array_action);

namespace qubus
{

local_object_factory_server::local_object_factory_server(local_address_space* address_space_)
: address_space_(address_space_)
{
}

namespace
{
void construct_object(void* base_ptr, const object_layout& layout, const abi_info& abi)
{
    auto location = static_cast<char*>(base_ptr) + layout.position;
    const auto& desc = layout.description;

    if (auto array_desc = desc.try_as<array_description>())
    {
        const auto& shape = array_desc->shape();

        new (location) util::index_t(array_desc->rank());

        auto* array_shape = static_cast<util::index_t*>(static_cast<void*>(location + sizeof(util::index_t)));

        for (std::size_t i = 0; i < shape.size(); ++i)
        {
            array_shape[i] = shape[i];
        }
    }

    if (auto struct_desc = desc.try_as<struct_description>())
    {
        auto members = static_cast<long int*>(static_cast<void*>(location));

        for (const auto& index_and_partition : boost::adaptors::index(layout.partitions))
        {
            const auto& i = index_and_partition.index();
            const auto& partition = index_and_partition.value();

            members[i] = partition.position - layout.position;
        }

        for (const auto& partition : layout.partitions)
        {
            construct_object(base_ptr, partition, abi);
        }
    }
}
}

hpx::future<hpx::id_type> local_object_factory_server::create_native_object(object_description description)
{
    auto layout = compute_layout(description, abi_);

    auto size = layout.size;
    auto alignment = sizeof(void*);

    auto instance = address_space_->allocate_page(size, alignment);

    auto base_ptr = instance.data().ptr();

    construct_object(base_ptr, layout, abi_);

    auto data_type = compute_type(description);

    auto obj = hpx::local_new<object>(data_type, size, instance);

    address_space_->register_page(obj, instance);

    return hpx::make_ready_future(obj.get());
}

hpx::future<hpx::id_type> local_object_factory_server::create_scalar(type data_type)
{
    return create_native_object(scalar_description(std::move(data_type)));
}

hpx::future<hpx::id_type>
local_object_factory_server::create_array(type value_type, std::vector<util::index_t> shape)
{
    return create_native_object(array_description(std::move(value_type), std::move(shape)));
}

local_object_factory::local_object_factory(hpx::id_type id)
: base_type(std::move(id))
{
}

local_object_factory::local_object_factory(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

object local_object_factory::create_native_object(object_description description)
{
    // FIXME: Reevaluate the impact of the manual unwrapping of the future.
    return hpx::async<local_object_factory_server::create_native_object_action>(
        this->get_id(), std::move(description)).get();
}

object local_object_factory::create_scalar(type data_type)
{
    // FIXME: Reevaluate the impact of the manual unwrapping of the future.
    return hpx::async<local_object_factory_server::create_scalar_action>(this->get_id(),
                                                                         std::move(data_type)).get();
}

object local_object_factory::create_array(type value_type, std::vector<util::index_t> shape)
{
    // FIXME: Reevaluate the impact of the manual unwrapping of the future.
    return hpx::async<local_object_factory_server::create_array_action>(
        this->get_id(), std::move(value_type), std::move(shape)).get();
}
}