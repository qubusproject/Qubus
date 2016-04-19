#include <qbb/qubus/index.hpp>

#include <utility>

using server_type = hpx::components::component<qbb::qubus::id_type_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_id_type_server);

namespace qbb
{
namespace qubus
{

id_type::id_type(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

}
}

