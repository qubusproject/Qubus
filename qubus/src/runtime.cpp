#include <qbb/qubus/runtime.hpp>

namespace qbb
{
namespace qubus
{

runtime_client::runtime_client(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

runtime::runtime() : client_(hpx::new_<runtime_server>(hpx::find_here()))
{
}
}
}

using server_type = hpx::components::component<qbb::qubus::runtime_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_runtime_server);
