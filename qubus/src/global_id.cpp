#include <qubus/global_id.hpp>

#include <utility>

using server_type = hpx::components::component<qubus::global_id_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_global_id_server);

namespace qubus
{

global_id::global_id(hpx::id_type id) : base_type(std::move(id))
{
}

global_id::global_id(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

global_id generate_global_id()
{
    return hpx::local_new<global_id>();
}

bool operator==(const global_id& lhs, const global_id& rhs)
{
    return lhs.get_id() == rhs.get_id();
}

bool operator!=(const global_id& lhs, const global_id& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const global_id& lhs, const global_id& rhs)
{
    return lhs.get_id() < rhs.get_id();
}
}