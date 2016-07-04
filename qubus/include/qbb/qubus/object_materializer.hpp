#ifndef QBB_QUBUS_OBJECT_MATERIALIZEER_HPP
#define QBB_QUBUS_OBJECT_MATERIALIZEER_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/local_address_space.hpp>
#include <qbb/qubus/object.hpp>

#include <hpx/include/lcos.hpp>

#include <tuple>
#include <vector>

namespace qbb
{
namespace qubus
{

hpx::future<std::tuple<void*, std::vector<local_address_space::pin>>>
materialize_object(object obj, local_address_space& addr_space);

hpx::future<std::vector<address_space::pin>>
        materialize_object(object obj, address_space& addr_space);
}
}

#endif