#ifndef QUBUS_GLOBAL_ID_HPP
#define QUBUS_GLOBAL_ID_HPP

#include <hpx/include/components.hpp>

namespace qubus
{

class global_id_server : public hpx::components::component_base<global_id_server>
{
};

class global_id : public hpx::components::client_base<global_id, global_id_server>
{
public:
    using base_type = hpx::components::client_base<global_id, global_id_server>;

    global_id() = default;

    global_id(hpx::id_type id);
    global_id(hpx::future<hpx::id_type>&& id);
};

bool operator==(const global_id& lhs, const global_id& rhs);
bool operator!=(const global_id& lhs, const global_id& rhs);

bool operator<(const global_id& lhs, const global_id& rhs);

global_id generate_global_id();

}

#endif
