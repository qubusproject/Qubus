#ifndef QBB_QUBUS_LOCAL_RUNTIME_HPP
#define QBB_QUBUS_LOCAL_RUNTIME_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/vpu.hpp>
#include <qbb/qubus/scheduler.hpp>

#include <qbb/qubus/abi_info.hpp>

#include <qbb/qubus/computelet.hpp>
#include <qbb/qubus/execution_context.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>

#include <qbb/qubus/local_address_space.hpp>
#include <qbb/qubus/local_object_factory.hpp>
#include <qbb/qubus/backend_registry.hpp>
#include <qbb/qubus/host_backend.hpp>

#include <hpx/include/components.hpp>

#include <boost/dll.hpp>

#include <memory>
#include <utility>

namespace qbb
{
namespace qubus
{

class local_runtime_server : public hpx::components::component_base<local_runtime_server>
{
public:
    local_runtime_server();
    ~local_runtime_server();

    local_object_factory get_local_object_factory() const;

    vpu get_local_vpu() const;

    const abi_info& get_abi_info()
    {
        return abi_info_;
    }

    HPX_DEFINE_COMPONENT_ACTION(local_runtime_server, get_local_object_factory, get_local_object_factory_action);
    HPX_DEFINE_COMPONENT_ACTION(local_runtime_server, get_local_vpu, get_local_vpu_action);
private:
    abi_info abi_info_;
    //boost::dll::shared_library cpu_plugin_;
    
    host_backend* cpu_backend_;

    backend_registry backend_registry_;

    std::unique_ptr<local_address_space> address_space_;
    local_object_factory object_factory_;

    vpu local_vpu_;
};

class local_runtime : public hpx::components::client_base<local_runtime, local_runtime_server>
{
public:
    using base_type = hpx::components::client_base<local_runtime, local_runtime_server>;

    local_runtime() = default;

    local_runtime(hpx::future<hpx::id_type>&& id);

    local_object_factory get_local_object_factory() const;

    vpu get_local_vpu() const;
};

local_runtime init_local_runtime();
HPX_DEFINE_PLAIN_ACTION(init_local_runtime, init_local_runtime_action);

local_runtime get_local_runtime();
HPX_DEFINE_PLAIN_ACTION(get_local_runtime, get_local_runtime_action);

}
}


#endif