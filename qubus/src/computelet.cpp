#include <qbb/qubus/computelet.hpp>

#include <qbb/qubus/IR/qir.hpp>

#include <utility>

using server_type = hpx::components::component<qbb::qubus::computelet_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_computelet_server);

typedef qbb::qubus::computelet_server::code_action code_action;
HPX_REGISTER_ACTION_DECLARATION(code_action);
HPX_REGISTER_ACTION(code_action);

namespace qbb
{
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

computelet make_foreign_computelet(foreign_computelet computelet)
{
    std::vector<variable_declaration> params;

    for (const auto& argument_type : computelet.argument_types())
    {
        params.emplace_back(argument_type);
    }

    variable_declaration result(computelet.result_type());

    std::vector<expression> args;

    for (const auto& param : params)
    {
        args.push_back(variable_ref_expr(param));
    }

    args.push_back(variable_ref_expr(result));

    expression body = foreign_call_expr(computelet, args);

    function_declaration decl("foreign_computelet", params, result, body);

    return make_computelet(decl);
}

}
}
