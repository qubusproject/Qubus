#ifndef QBB_QUBUS_LOCAL_OBJECT_FACTORY_HPP
#define QBB_QUBUS_LOCAL_OBJECT_FACTORY_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/object.hpp>
#include <qbb/qubus/local_address_space.hpp>

#include <qbb/qubus/abi_info.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <hpx/include/components.hpp>

#include <qbb/util/integers.hpp>

#include <vector>

namespace qbb
{
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

    object_client create_array(type value_type, std::vector<util::index_t> shape);

    HPX_DEFINE_COMPONENT_ACTION(local_object_factory_server, create_array, create_array_action);

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

    object_client create_array(type value_type, std::vector<util::index_t> shape);
};
}
}

#endif