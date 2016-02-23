#ifndef QBB_QUBUS_RUNTIME_HPP
#define QBB_QUBUS_RUNTIME_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/local_runtime.hpp>

#include <qbb/qubus/computelet.hpp>
#include <qbb/qubus/execution_context.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/actions.hpp>

namespace qbb
{
namespace qubus
{

class runtime_server : public hpx::components::component_base<runtime_server>
{
public:
    void execute(computelet c);

    HPX_DEFINE_COMPONENT_ACTION(runtime_server, execute, execute_action);

private:
    local_runtime runtime_;
};

class runtime : public hpx::components::client_base<runtime, runtime_server>
{
public:
    using base_type = hpx::components::client_base<runtime, runtime_server>;

    runtime(hpx::future<hpx::id_type>&& id);

    // object_factory& get_object_factory();

    // plan register_user_defined_plan(user_defined_plan_t plan); Is this still necessary?

    void execute(computelet c);

    // const abi_info& abi();
};

}
}

#endif
