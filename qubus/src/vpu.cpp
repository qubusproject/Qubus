#include <qbb/qubus/vpu.hpp>

#include <utility>

using server_type = hpx::components::component<qbb::qubus::remote_vpu_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_remote_vpu_server);

using execute_action = qbb::qubus::remote_vpu_server::execute_action;
HPX_REGISTER_ACTION_DECLARATION(execute_action, remote_vpu_server_execute_action);
HPX_REGISTER_ACTION(execute_action, remote_vpu_server_execute_action);

using view_server_type = hpx::components::component<qbb::qubus::remote_vpu_reference_server>;
HPX_REGISTER_COMPONENT(view_server_type, qbb_qubus_remote_vpu_reference_server);

using ref_execute_action = qbb::qubus::remote_vpu_reference_server::execute_action;
HPX_REGISTER_ACTION_DECLARATION(ref_execute_action, remote_vpu_reference_server_execute_action);
HPX_REGISTER_ACTION(ref_execute_action, remote_vpu_reference_server_execute_action);

namespace qbb
{
namespace qubus
{

remote_vpu_server::remote_vpu_server(std::unique_ptr<vpu> underlying_vpu_)
: underlying_vpu_(std::move(underlying_vpu_))
{
}

void remote_vpu_server::execute(computelet c, execution_context ctx) const
{
    underlying_vpu_->execute(std::move(c), std::move(ctx)).get();
}

remote_vpu::remote_vpu(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

hpx::future<void> remote_vpu::execute(computelet c, execution_context ctx) const
{
    return hpx::async<remote_vpu_server::execute_action>(this->get_id(), std::move(c),
                                                         std::move(ctx));
}

remote_vpu_reference_server::remote_vpu_reference_server(vpu* underlying_vpu_)
: underlying_vpu_(underlying_vpu_)
{
}

void remote_vpu_reference_server::execute(computelet c, execution_context ctx) const
{
    underlying_vpu_->execute(std::move(c), std::move(ctx)).get();
}

remote_vpu_reference::remote_vpu_reference(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

hpx::future<void> remote_vpu_reference::execute(computelet c, execution_context ctx) const
{
    return hpx::async<remote_vpu_reference_server::execute_action>(this->get_id(), std::move(c),
                                                                   std::move(ctx));
}
}
}