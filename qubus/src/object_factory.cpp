#include <qbb/qubus/object_factory.hpp>

#include <qbb/util/make_unique.hpp>
#include <qbb/util/integers.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

void object_factory::add_factory_and_addr_space(const std::string& id,
                                                local_object_factory& factory,
                                                local_address_space& address_space)
{
    local_factories_[id] = &factory;
    address_spaces_[id] = &address_space;
}

std::unique_ptr<local_array> object_factory::create_array(type value_type,
                                                          std::vector<util::index_t> shape)
{
    // TODO: Reject arrays with a rank of 0

    auto factory = local_factories_.at("qubus.cpu");

    auto mem_block = factory->create_array(std::move(value_type), shape);

    auto address = handle_fac_.create();

    auto& address_space = address_spaces_.at("qubus.cpu");

    address_space->register_mem_block(address, std::move(mem_block));

    return util::make_unique<local_array>(address, value_type, shape);
}

std::unique_ptr<local_tensor> object_factory::create_tensor(type value_type,
                                                            std::vector<util::index_t> shape)
{
    return util::make_unique<local_tensor>(create_array(value_type, shape));
}

std::unique_ptr<struct_> object_factory::create_struct(type struct_type,
                                                       std::vector<std::unique_ptr<object>> members)
{
    return util::make_unique<struct_>(struct_type, std::move(members));
}

std::unique_ptr<struct_>
object_factory::create_sparse_tensor(type value_type, std::vector<util::index_t> shape, const sparse_tensor_layout& layout)
{
    auto shape_array = create_array(types::integer(), {util::to_uindex(shape.size())});

    auto sell_tensor_type = types::struct_(
        "sell_tensor", {types::struct_::member(types::array(value_type), "val"), types::struct_::member(types::array(types::integer()), "col"),
                        types::struct_::member(types::array(types::integer()), "cs"), types::struct_::member(types::array(types::integer()), "cl")});

    std::unique_ptr<object> sell_tensor_val = create_array(value_type, {layout.nnz});
    std::unique_ptr<object> sell_tensor_col = create_array(types::integer(), {layout.nnz});
    std::unique_ptr<object> sell_tensor_cs = create_array(types::integer(), {layout.num_chunks+1});
    std::unique_ptr<object> sell_tensor_cl = create_array(types::integer(), {layout.num_chunks});

    std::vector<std::unique_ptr<object>> sell_tensor_members;

    sell_tensor_members.push_back(std::move(sell_tensor_val));
    sell_tensor_members.push_back(std::move(sell_tensor_col));
    sell_tensor_members.push_back(std::move(sell_tensor_cs));
    sell_tensor_members.push_back(std::move(sell_tensor_cl));

    auto sell_tensor = create_struct(sell_tensor_type, std::move(sell_tensor_members));

    auto sparse_tensor_type = types::struct_("sparse_tensor", {types::struct_::member(sell_tensor_type, "data"), types::struct_::member(types::array(types::integer()), "shape")});

    std::vector<std::unique_ptr<object>> sparse_tensor_members;

    sparse_tensor_members.push_back(std::move(sell_tensor));
    sparse_tensor_members.push_back(std::move(shape_array));

    return create_struct(sparse_tensor_type, std::move(sparse_tensor_members));
}
}
}