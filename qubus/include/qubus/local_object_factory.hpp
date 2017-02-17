#ifndef QUBUS_LOCAL_OBJECT_FACTORY_HPP
#define QUBUS_LOCAL_OBJECT_FACTORY_HPP

#include <hpx/config.hpp>

#include <qubus/object.hpp>
#include <qubus/local_address_space.hpp>

#include <qubus/abi_info.hpp>

#include <qubus/IR/type.hpp>

#include <hpx/include/components.hpp>

#include <qubus/util/integers.hpp>

#include <vector>

namespace qubus
{

class local_object_factory_server
    : public hpx::components::component_base<local_object_factory_server>
{
public:
    local_object_factory_server() = default;

    // Note: This function takes a pointer instead of a reference since HPX can not currently handle
    // reference arguments.
    explicit local_object_factory_server(local_address_space* address_space_);

    local_object_factory_server(const local_object_factory_server&) = delete;
    local_object_factory_server& operator=(const local_object_factory_server&) = delete;

    hpx::future<hpx::id_type> create_scalar(type data_type);
    hpx::future<hpx::id_type> create_array(type value_type, std::vector<util::index_t> shape);
    hpx::future<hpx::id_type> create_struct(type struct_type, std::vector<object> members);

    HPX_DEFINE_COMPONENT_ACTION(local_object_factory_server, create_scalar, create_scalar_action);
    HPX_DEFINE_COMPONENT_ACTION(local_object_factory_server, create_array, create_array_action);
    HPX_DEFINE_COMPONENT_ACTION(local_object_factory_server, create_struct, create_struct_action);
private:
    local_address_space* address_space_;
    abi_info abi_;
};

class local_object_factory
    : public hpx::components::client_base<local_object_factory, local_object_factory_server>
{
public:
    using base_type =
        hpx::components::client_base<local_object_factory, local_object_factory_server>;

    local_object_factory() = default;
    local_object_factory(hpx::future<hpx::id_type>&& id);

    object create_scalar(type data_type);
    object create_array(type value_type, std::vector<util::index_t> shape);
    object create_struct(type struct_type, std::vector<object> members);
};
}

#endif