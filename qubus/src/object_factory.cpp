#include <qbb/qubus/object_factory.hpp>

#include <qbb/qubus/primitive_object.hpp>

#include <qbb/util/integers.hpp>

#include <utility>

using server_type = hpx::components::component<qbb::qubus::object_factory_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_object_factory_server);

typedef qbb::qubus::object_factory_server::create_array_action create_array_action;
HPX_REGISTER_ACTION_DECLARATION(create_array_action);
HPX_REGISTER_ACTION(create_array_action);

namespace qbb
{
namespace qubus
{

object_factory_server::object_factory_server(abi_info abi_, std::vector<local_runtime> local_runtimes_)
: abi_(std::move(abi_))
{
    for (const auto& runtime : local_runtimes_)
    {
        local_factories_.push_back(runtime.get_local_object_factory());
    }
}

object_client object_factory_server::create_array(type value_type, std::vector<util::index_t> shape)
{
    return local_factories_.at(0).create_array(std::move(value_type), std::move(shape));
}

/*std::unique_ptr<local_array> object_factory::create_array(type value_type,
                                                          std::vector<util::index_t> shape)
{
    // TODO: Reject arrays with a rank of 0

    auto size_type_size = abi_.get_size_of(types::integer());

    auto layout = abi_.get_array_layout(value_type, shape);

    auto rank = util::to_uindex(shape.size());

    auto mem_block = allocator_->allocate(layout.size(), layout.alignment());

    char* ptr = static_cast<char*>(mem_block->ptr());

    char* shape_ptr = ptr + layout.shape_offset();

    std::memcpy(ptr, &rank, size_type_size);

    std::memcpy(shape_ptr, shape.data(), layout.shape_size());

    return std::make_unique<local_array>(std::move(mem_block), value_type, shape);
}

std::unique_ptr<local_tensor> object_factory::create_tensor(type value_type,
                                                            std::vector<util::index_t> shape)
{
    return std::make_unique<local_tensor>(create_array(value_type, shape));
}

std::unique_ptr<struct_> object_factory::create_struct(type struct_type,
                                                       std::vector<std::unique_ptr<object>> members)
{
    return std::make_unique<struct_>(struct_type, std::move(members));
}

std::unique_ptr<struct_> object_factory::create_sparse_tensor(type value_type,
                                                              std::vector<util::index_t> shape,
                                                              const sparse_tensor_layout& layout)
{
    auto shape_array = create_array(types::integer(), {util::to_uindex(shape.size())});

    auto sell_tensor_type = types::struct_(
        "sell_tensor", {types::struct_::member(types::array(value_type), "val"),
                        types::struct_::member(types::array(types::integer()), "col"),
                        types::struct_::member(types::array(types::integer()), "cs"),
                        types::struct_::member(types::array(types::integer()), "cl")});

    std::unique_ptr<object> sell_tensor_val = create_array(value_type, {layout.nnz});
    std::unique_ptr<object> sell_tensor_col = create_array(types::integer(), {layout.nnz});
    std::unique_ptr<object> sell_tensor_cs =
        create_array(types::integer(), {layout.num_chunks + 1});
    std::unique_ptr<object> sell_tensor_cl = create_array(types::integer(), {layout.num_chunks});

    std::vector<std::unique_ptr<object>> sell_tensor_members;

    sell_tensor_members.push_back(std::move(sell_tensor_val));
    sell_tensor_members.push_back(std::move(sell_tensor_col));
    sell_tensor_members.push_back(std::move(sell_tensor_cs));
    sell_tensor_members.push_back(std::move(sell_tensor_cl));

    auto sell_tensor = create_struct(sell_tensor_type, std::move(sell_tensor_members));

    auto sparse_tensor_type = types::struct_(
        "sparse_tensor", {types::struct_::member(sell_tensor_type, "data"),
                          types::struct_::member(types::array(types::integer()), "shape")});

    std::vector<std::unique_ptr<object>> sparse_tensor_members;

    sparse_tensor_members.push_back(std::move(sell_tensor));
    sparse_tensor_members.push_back(std::move(shape_array));

    return create_struct(sparse_tensor_type, std::move(sparse_tensor_members));
}*/

object_factory::object_factory(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

object_client object_factory::create_array(type value_type, std::vector<util::index_t> shape)
{
    return hpx::async<object_factory_server::create_array_action>(
               this->get_id(), std::move(value_type), std::move(shape))
        .get();
}
}
}