#ifndef QBB_QUBUS_RUNTIME_HPP
#define QBB_QUBUS_RUNTIME_HPP

#include <hpx/include/components.hpp>
#include <hpx/include/actions.hpp>

namespace qbb
{
namespace qubus
{

class runtime_server : public hpx::components::component_base<runtime_server>
{
public:
};

class runtime_client : public hpx::components::client_base<runtime_client, runtime_server>
{
public:
    using base_type = hpx::components::client_base<runtime_client, runtime_server>;

    runtime_client(hpx::future<hpx::id_type>&& id);
};

class runtime
{
public:
    runtime();

    // object_factory& get_object_factory();

    // plan register_user_defined_plan(user_defined_plan_t plan); Is this still necessary?

    // void execute(computelet c, execution_context ctx);

    // const abi_info& abi();
private:
    runtime_client client_;
};
}
}

#endif
