#ifndef QUBUS_LOCAL_RUNTIME_HPP
#define QUBUS_LOCAL_RUNTIME_HPP

#include <hpx/config.hpp>

#include <qubus/aggregate_vpu.hpp>
#include <qubus/virtual_address_space.hpp>

#include <qubus/abi_info.hpp>

#include <qubus/IR/function_declaration.hpp>
#include <qubus/computelet.hpp>
#include <qubus/execution_context.hpp>

#include <qubus/backend_registry.hpp>
#include <qubus/host_backend.hpp>
#include <qubus/local_address_space.hpp>
#include <qubus/local_object_factory.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/thread_executors.hpp>

#include <boost/filesystem.hpp>

#include <memory>
#include <utility>

namespace qubus
{

class local_runtime
{
public:
    local_runtime(std::unique_ptr<virtual_address_space> global_address_space_);

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

    auto& get_service_executor()
    {
        return service_executor_;
    }

private:
    void try_to_load_host_backend(const boost::filesystem::path& library_path);
    void try_to_load_backend(const boost::filesystem::path& library_path,
                             host_backend& the_host_backend);
    void scan_for_backends();

    hpx::future<local_address_space::address_entry>
    resolve_page_fault(const object& obj, local_address_space::page_fault_context ctx);

    hpx::threads::executors::local_priority_queue_os_executor service_executor_;

    abi_info abi_info_;

    backend_registry backend_registry_;

    std::unique_ptr<virtual_address_space> global_address_space_;

    std::unique_ptr<local_address_space> address_space_;
    local_object_factory object_factory_;

    std::unique_ptr<vpu> local_vpu_;
};

class local_runtime_reference_server
    : public hpx::components::component_base<local_runtime_reference_server>
{
public:
    local_runtime_reference_server() = default;
    explicit local_runtime_reference_server(local_runtime* runtime_);

    hpx::future<hpx::id_type> get_local_object_factory() const;

    std::unique_ptr<remote_vpu_reference> get_local_vpu() const;

    HPX_DEFINE_COMPONENT_ACTION(local_runtime_reference_server, get_local_object_factory,
                                get_local_object_factory_action);
    HPX_DEFINE_COMPONENT_ACTION(local_runtime_reference_server, get_local_vpu,
                                get_local_vpu_action);

private:
    local_runtime* runtime_;
};

class local_runtime_reference
    : public hpx::components::client_base<local_runtime_reference, local_runtime_reference_server>
{
public:
    using base_type =
        hpx::components::client_base<local_runtime_reference, local_runtime_reference_server>;

    local_runtime_reference() = default;

    local_runtime_reference(hpx::future<hpx::id_type>&& id);

    local_object_factory get_local_object_factory() const;

    std::unique_ptr<remote_vpu_reference> get_local_vpu() const;
};

local_runtime& init_local_runtime(virtual_address_space_wrapper::client global_addr_space);
void shutdown_local_runtime();
local_runtime& get_local_runtime();

local_runtime_reference
init_local_runtime_on_locality(const hpx::id_type& locality,
                               virtual_address_space_wrapper::client global_addr_space);
void shutdown_local_runtime_on_locality(const hpx::id_type& locality);

local_runtime_reference get_local_runtime_on_locality(const hpx::id_type& locality);
}

#endif