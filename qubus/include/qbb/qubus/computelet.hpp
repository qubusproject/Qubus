#ifndef QUBUS_COMPUTELET_HPP
#define QUBUS_COMPUTELET_HPP

#include <qbb/qubus/IR/function_declaration.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/actions.hpp>

namespace qubus
{

class computelet_server : public hpx::components::component_base<computelet_server>
{
public:
    computelet_server() = default; // Create an invalid computelet.
    explicit computelet_server(function_declaration code_);

    function_declaration code() const;

    HPX_DEFINE_COMPONENT_ACTION(computelet_server, code, code_action);
private:
    function_declaration code_;
};

class computelet : public hpx::components::client_base<computelet, computelet_server>
{
public:
    using base_type = hpx::components::client_base<computelet, computelet_server>;

    computelet() = default;
    computelet(hpx::future<hpx::id_type>&& id);

    hpx::future<function_declaration> code() const;

    const hpx::naming::gid_type& id() const;
};

computelet make_computelet(function_declaration code);

}

#endif
