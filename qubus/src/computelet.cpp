#include <qubus/computelet.hpp>

#include <qubus/IR/qir.hpp>

#include <utility>

using server_type = hpx::components::component<qubus::computelet_server>;
HPX_REGISTER_COMPONENT(server_type, QUBUS_computelet_server);

typedef qubus::computelet_server::code_action code_action;
HPX_REGISTER_ACTION_DECLARATION(code_action);
HPX_REGISTER_ACTION(code_action);

namespace qubus
{

computelet_server::computelet_server(function_declaration code_)
: code_(std::move(code_))
{
}

function_declaration computelet_server::code() const
{
    return code_;
}

computelet::computelet(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

hpx::future<function_declaration> computelet::code() const
{
    return hpx::async<computelet_server::code_action>(this->get_id());
}

const hpx::naming::gid_type& computelet::id() const
{
    return this->get_raw_gid();
}

computelet make_computelet(function_declaration code)
{
    return hpx::new_<computelet_server>(hpx::find_here(), std::move(code));
}

}
