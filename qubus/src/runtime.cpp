#include <qbb/qubus/runtime.hpp>

#include <qbb/util/unused.hpp>

using server_type = hpx::components::component<qbb::qubus::runtime_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_runtime_server);

typedef qbb::qubus::runtime_server::execute_action execute_action;
HPX_REGISTER_ACTION_DECLARATION(execute_action);
HPX_REGISTER_ACTION(execute_action);

namespace qbb
{
namespace qubus
{

void runtime_server::execute(computelet c)
{

}

runtime::runtime(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

void runtime::execute(computelet c)
{
    hpx::apply<runtime_server::execute_action>(this->get_id(), std::move(c));
}

namespace
{
runtime qubus_runtime = {};
std::once_flag qubus_runtime_init_flag;
}

void init_new(int QBB_UNUSED(argc), char** QBB_UNUSED(argv))
{
    std::call_once(qubus_runtime_init_flag, []
                   {
                       qubus_runtime = hpx::new_<runtime>(hpx::find_here());
                       hpx::agas::register_name("/qubus/runtime", qubus_runtime.get_gid());
                   });
}

}
}
