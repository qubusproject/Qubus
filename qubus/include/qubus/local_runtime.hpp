#ifndef QUBUS_LOCAL_RUNTIME_HPP
#define QUBUS_LOCAL_RUNTIME_HPP

#include <hpx/config.hpp>

#include <qubus/aggregate_vpu.hpp>

#include <qubus/abi_info.hpp>

#include <qubus/computelet.hpp>
#include <qubus/execution_context.hpp>
#include <qubus/IR/function_declaration.hpp>

#include <qubus/local_address_space.hpp>
#include <qubus/local_object_factory.hpp>
#include <qubus/backend_registry.hpp>
#include <qubus/host_backend.hpp>

#include <hpx/include/components.hpp>

#include <boost/dll.hpp>

#include <memory>
#include <utility>

namespace qubus
{

class local_runtime
{
public:
    local_runtime();

    local_runtime(const local_runtime&) = delete;
    local_runtime& operator=(const local_runtime&) = delete;

    local_runtime(local_runtime&&) = delete;
    local_runtime& operator=(local_runtime&&) = delete;

    local_object_factory get_local_object_factory() const;
    vpu& get_local_vpu() const;
    local_address_space& get_address_space() const;

    const abi_info& get_abi_info()
    {
        return abi_info_;
    }

private:
    abi_info abi_info_;
    //boost::dll::shared_library cpu_plugin_;

    host_backend* cpu_backend_;

    backend_registry backend_registry_;

    std::unique_ptr<local_address_space> address_space_;
    local_object_factory object_factory_;

    std::unique_ptr<vpu> local_vpu_;
};

class local_runtime_reference_server : public hpx::components::component_base<local_runtime_reference_server>
{
public:
    local_runtime_reference_server() = default;
    explicit local_runtime_reference_server(local_runtime* runtime_);

    hpx::future<hpx::id_type> get_local_object_factory() const;

    std::unique_ptr<remote_vpu_reference> get_local_vpu() const;

    HPX_DEFINE_COMPONENT_ACTION(local_runtime_reference_server, get_local_object_factory, get_local_object_factory_action);
    HPX_DEFINE_COMPONENT_ACTION(local_runtime_reference_server, get_local_vpu, get_local_vpu_action);
private:
    local_runtime* runtime_;
};

class local_runtime_reference : public hpx::components::client_base<local_runtime_reference, local_runtime_reference_server>
{
public:
    using base_type = hpx::components::client_base<local_runtime_reference, local_runtime_reference_server>;

    local_runtime_reference() = default;

    local_runtime_reference(hpx::future<hpx::id_type>&& id);

    local_object_factory get_local_object_factory() const;

    std::unique_ptr<remote_vpu_reference> get_local_vpu() const;
};

local_runtime& init_local_runtime();
local_runtime& get_local_runtime();

local_runtime_reference init_local_runtime_on_locality(const hpx::id_type& locality);
local_runtime_reference get_local_runtime_on_locality(const hpx::id_type& locality);

}


#endif