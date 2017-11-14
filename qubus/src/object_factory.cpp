#include <qubus/object_factory.hpp>

#include <qubus/util/integers.hpp>

#include <utility>

using server_type = hpx::components::component<qubus::object_factory_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_object_factory_server);

using create_scalar_action = qubus::object_factory_server::create_scalar_action;
HPX_REGISTER_ACTION(create_scalar_action, qubus_object_factory_create_scalar_action);

using create_array_action = qubus::object_factory_server::create_array_action;
HPX_REGISTER_ACTION(create_array_action, qubus_object_factory_create_array_action);

using create_sparse_tensor_action = qubus::object_factory_server::create_sparse_tensor_action;
HPX_REGISTER_ACTION(create_sparse_tensor_action, qubus_object_factory_create_sparse_tensor_action);

namespace qubus
{

object_factory_server::object_factory_server(abi_info abi_,
                                             std::vector<local_runtime_reference> local_runtimes_)
: abi_(std::move(abi_))
{
    for (const auto& runtime : local_runtimes_)
    {
        local_factories_.push_back(runtime.get_local_object_factory());
    }
}

hpx::future<hpx::id_type> object_factory_server::create_scalar(type data_type)
{
    static long int next_factory = 0;

    auto obj = local_factories_.at(next_factory).create_scalar(std::move(data_type));

    next_factory = (next_factory + 1) % hpx::get_num_localities(hpx::launch::sync);

    return hpx::make_ready_future(obj.get());
}

hpx::future<hpx::id_type> object_factory_server::create_array(type value_type,
                                                              std::vector<util::index_t> shape)
{
    static long int next_factory = 0;

    auto obj =
        local_factories_.at(next_factory).create_array(std::move(value_type), std::move(shape));

    next_factory = (next_factory + 1) % hpx::get_num_localities(hpx::launch::sync);

    return hpx::make_ready_future(obj.get());
}

hpx::future<hpx::id_type>
object_factory_server::create_sparse_tensor(type value_type, std::vector<util::index_t> shape,
                                            const sparse_tensor_layout& layout)
{
    /*auto choosen_factory = local_factories_.at(0);

    auto shape_array =
        choosen_factory.create_array(types::integer(), {util::to_uindex(shape.size())});

    auto sell_tensor_type = types::struct_(
        "sell_tensor", {types::struct_::member(types::array(value_type, 1), "val"),
                        types::struct_::member(types::array(types::integer(), 1), "col"),
                        types::struct_::member(types::array(types::integer(), 1), "cs"),
                        types::struct_::member(types::array(types::integer(), 1), "cl")});

    object sell_tensor_val = choosen_factory.create_array(value_type, {layout.nnz});
    object sell_tensor_col = choosen_factory.create_array(types::integer(), {layout.nnz});
    object sell_tensor_cs = choosen_factory.create_array(types::integer(), {layout.num_chunks + 1});
    object sell_tensor_cl = choosen_factory.create_array(types::integer(), {layout.num_chunks});

    std::vector<object> sell_tensor_members;

    sell_tensor_members.push_back(std::move(sell_tensor_val));
    sell_tensor_members.push_back(std::move(sell_tensor_col));
    sell_tensor_members.push_back(std::move(sell_tensor_cs));
    sell_tensor_members.push_back(std::move(sell_tensor_cl));

    auto sell_tensor =
        choosen_factory.create_struct(sell_tensor_type, std::move(sell_tensor_members));

    auto sparse_tensor_type = types::struct_(
        "sparse_tensor", {types::struct_::member(sell_tensor_type, "data"),
                          types::struct_::member(types::array(types::integer(), 1), "shape")});

    std::vector<object> sparse_tensor_members;

    sparse_tensor_members.push_back(std::move(sell_tensor));
    sparse_tensor_members.push_back(std::move(shape_array));

    auto sparse_tensor =
        choosen_factory.create_struct(sparse_tensor_type, std::move(sparse_tensor_members));

    return hpx::make_ready_future(sparse_tensor.get());*/

    std::terminate(); // TODO: Reimplement this.
}

object_factory::object_factory(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

object object_factory::create_scalar(type data_type)
{
    // FIXME: Reevaluate the impact of the manual unwrapping of the future.
    return hpx::async<object_factory_server::create_scalar_action>(
            this->get_id(), std::move(data_type)).get();
}

object object_factory::create_array(type value_type, std::vector<util::index_t> shape)
{
    // FIXME: Reevaluate the impact of the manual unwrapping of the future.
    return hpx::async<object_factory_server::create_array_action>(
        this->get_id(), std::move(value_type), std::move(shape)).get();
}

object object_factory::create_sparse_tensor(type value_type, std::vector<util::index_t> shape,
                                            const sparse_tensor_layout& layout)
{
    // FIXME: Reevaluate the impact of the manual unwrapping of the future.
    return hpx::async<object_factory_server::create_sparse_tensor_action>(
        this->get_id(), std::move(value_type), std::move(shape), std::move(layout)).get();
}
}