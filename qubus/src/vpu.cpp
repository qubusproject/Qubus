#include <qubus/vpu.hpp>

#include <utility>

using server_type = hpx::components::component<qubus::remote_vpu_server>;
HPX_REGISTER_COMPONENT(server_type, QUBUS_remote_vpu_server);

using execute_action = qubus::remote_vpu_server::execute_action;
HPX_REGISTER_ACTION_DECLARATION(execute_action, remote_vpu_server_execute_action);
HPX_REGISTER_ACTION(execute_action, remote_vpu_server_execute_action);

using try_estimate_execution_time_action =
    qubus::remote_vpu_server::try_estimate_execution_time_action;
HPX_REGISTER_ACTION_DECLARATION(try_estimate_execution_time_action,
                                remote_vpu_server_try_estimate_execution_time_action);
HPX_REGISTER_ACTION(try_estimate_execution_time_action,
                    remote_vpu_server_try_estimate_execution_time_action);

using view_server_type = hpx::components::component<qubus::remote_vpu_reference_server>;
HPX_REGISTER_COMPONENT(view_server_type, QUBUS_remote_vpu_reference_server);

using ref_execute_action = qubus::remote_vpu_reference_server::execute_action;
HPX_REGISTER_ACTION_DECLARATION(ref_execute_action, remote_vpu_reference_server_execute_action);
HPX_REGISTER_ACTION(ref_execute_action, remote_vpu_reference_server_execute_action);

using ref_try_estimate_execution_time_action =
    qubus::remote_vpu_reference_server::try_estimate_execution_time_action;
HPX_REGISTER_ACTION_DECLARATION(ref_try_estimate_execution_time_action,
                                remote_vpu_reference_server_try_estimate_execution_time_action);
HPX_REGISTER_ACTION(ref_try_estimate_execution_time_action,
                    remote_vpu_reference_server_try_estimate_execution_time_action);

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

boost::optional<performance_estimate>
remote_vpu_server::try_estimate_execution_time(const computelet& c,
                                               const execution_context& ctx) const
{
    return underlying_vpu_->try_estimate_execution_time(c, ctx).get();
}

remote_vpu::remote_vpu(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

hpx::future<void> remote_vpu::execute(computelet c, execution_context ctx) const
{
    return hpx::async<remote_vpu_server::execute_action>(this->get_id(), std::move(c),
                                                         std::move(ctx));
}

hpx::future<boost::optional<performance_estimate>>
remote_vpu::try_estimate_execution_time(const computelet& c, const execution_context& ctx) const
{
    return hpx::async<remote_vpu_server::try_estimate_execution_time_action>(this->get_id(), c,
                                                                             ctx);
}

remote_vpu_reference_server::remote_vpu_reference_server(vpu* underlying_vpu_)
: underlying_vpu_(underlying_vpu_)
{
}

void remote_vpu_reference_server::execute(computelet c, execution_context ctx) const
{
    underlying_vpu_->execute(std::move(c), std::move(ctx)).get();
}

boost::optional<performance_estimate>
remote_vpu_reference_server::try_estimate_execution_time(const computelet& c,
                                                         const execution_context& ctx) const
{
    return underlying_vpu_->try_estimate_execution_time(c, ctx).get();
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

hpx::future<boost::optional<performance_estimate>>
remote_vpu_reference::try_estimate_execution_time(const computelet& c,
                                                  const execution_context& ctx) const
{
    return hpx::async<remote_vpu_reference_server::try_estimate_execution_time_action>(
        this->get_id(), c, ctx);
}
}
