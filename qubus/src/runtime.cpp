#include <qubus/runtime.hpp>

#include <qubus/basic_address_space.hpp>

#include <qubus/hpx_utils.hpp>

#include <qubus/util/unused.hpp>

using server_type = hpx::components::component<qubus::runtime_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_runtime_server);

typedef qubus::runtime_server::execute_action execute_action;
HPX_REGISTER_ACTION_DECLARATION(execute_action, runtime_server_execute_action);
HPX_REGISTER_ACTION(execute_action, runtime_server_execute_action);

typedef qubus::runtime_server::get_object_factory_action get_object_factory_action;
HPX_REGISTER_ACTION_DECLARATION(get_object_factory_action,
                                runtime_server_get_object_factory_action);
HPX_REGISTER_ACTION(get_object_factory_action, runtime_server_get_object_factory_action);

namespace qubus
{

runtime_server::runtime_server()
{
    for (const auto& locality : hpx::find_all_localities())
    {
        local_runtimes_.push_back(init_local_runtime_on_locality(locality));
    }

    auto addr_space_impl = std::make_unique<basic_address_space>();

    global_address_space_ = virtual_address_space_wrapper(std::move(addr_space_impl));

    obj_factory_ = hpx::new_<object_factory>(hpx::find_here(), abi_info(), local_runtimes_);

    global_vpu_ = aggregate_vpu(std::make_unique<round_robin_scheduler>());

    for (const auto& runtime : local_runtimes_)
    {
        global_vpu_->add_member_vpu(runtime.get_local_vpu());
    }
}

void runtime_server::execute(computelet c, execution_context ctx)
{
    global_vpu_->execute(std::move(c), std::move(ctx)).wait();
}

hpx::future<hpx::id_type> runtime_server::get_object_factory() const
{
    return hpx::make_ready_future(obj_factory_.get());
}

runtime::runtime(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

hpx::future<void> runtime::execute(computelet c, execution_context ctx)
{
    return hpx::async<runtime_server::execute_action>(this->get_id(), std::move(c), std::move(ctx));
}

object_factory runtime::get_object_factory() const
{
    return hpx::async<runtime_server::get_object_factory_action>(this->get_id());
}

void init(int QUBUS_UNUSED(argc), char** QUBUS_UNUSED(argv))
{
    auto global_runtime = hpx::agas::resolve_name(hpx::launch::sync, "/qubus/runtime");

    if (!global_runtime)
    {
        auto new_runtime = hpx::new_<runtime>(hpx::find_here());
        hpx::agas::register_name(hpx::launch::sync, "/qubus/runtime", new_runtime.get_id());
    }
}

runtime get_runtime()
{
    return hpx::agas::resolve_name("/qubus/runtime");
}
}
