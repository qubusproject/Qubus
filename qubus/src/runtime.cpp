#include <qbb/qubus/runtime.hpp>

#include <qbb/qubus/aggregate_vpu.hpp>

#include <qbb/qubus/hpx_utils.hpp>

#include <qbb/util/unused.hpp>

using server_type = hpx::components::component<qbb::qubus::runtime_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_runtime_server);

typedef qbb::qubus::runtime_server::execute_action execute_action;
HPX_REGISTER_ACTION_DECLARATION(execute_action);
HPX_REGISTER_ACTION(execute_action);

typedef qbb::qubus::runtime_server::get_object_factory_action get_object_factory_action;
HPX_REGISTER_ACTION_DECLARATION(get_object_factory_action);
HPX_REGISTER_ACTION(get_object_factory_action);

HPX_REGISTER_ACTION(qbb::qubus::get_runtime_action, qbb_qubus_get_runtime_action);

namespace qbb
{
namespace qubus
{

runtime_server::runtime_server()
{
    for (const auto& locality : hpx::find_all_localities())
    {
        local_runtimes_.push_back(hpx::async(qbb::qubus::init_local_runtime_action(), locality).get());
    }

    obj_factory_ = hpx::new_<object_factory>(hpx::find_here(), abi_info(), local_runtimes_);

    global_vpu_ = new_here<aggregate_vpu>();

    auto global_vpu_ptr = hpx::get_ptr_sync<aggregate_vpu>(global_vpu_.get_id());

    for (const auto& runtime : local_runtimes_)
    {
        global_vpu_ptr->add_member_vpu(runtime.get_local_vpu());
    }
}

void runtime_server::execute(computelet c, execution_context ctx)
{
    global_vpu_.execute(std::move(c), std::move(ctx));
}

object_factory runtime_server::get_object_factory() const
{
    return obj_factory_;
}

runtime::runtime(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

void runtime::execute(computelet c, execution_context ctx)
{
    hpx::apply<runtime_server::execute_action>(this->get_id(), std::move(c), std::move(ctx));
}

object_factory runtime::get_object_factory() const
{
    return hpx::async<runtime_server::get_object_factory_action>(this->get_id()).get();
}

namespace
{
runtime qubus_runtime = {};
std::once_flag qubus_runtime_init_flag;
}

void init(int QBB_UNUSED(argc), char** QBB_UNUSED(argv))
{
    std::call_once(qubus_runtime_init_flag, []
                   {
                       qubus_runtime = hpx::new_<runtime>(hpx::find_here());
                       hpx::agas::register_name("/qubus/runtime", qubus_runtime.get_gid());
                   });
}

runtime get_runtime()
{
    return qubus_runtime;
}

}
}
