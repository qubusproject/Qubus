#ifndef QUBUS_RUNTIME_HPP
#define QUBUS_RUNTIME_HPP

#include <hpx/config.hpp>

#include <qubus/local_runtime.hpp>

#include <qubus/block_address_allocator.hpp>
#include <qubus/aggregate_vpu.hpp>
#include <qubus/virtual_address_space.hpp>
#include <qubus/module_library.hpp>
#include <qubus/dataflow.hpp>

#include <qubus/IR/symbol_id.hpp>
#include <qubus/kernel_arguments.hpp>

#include <qubus/exception.hpp>

#include <boost/optional.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/actions.hpp>
#include <hpx/include/resource_partitioner.hpp>

#include <string>
#include <vector>

namespace qubus
{

class not_initialized_exception : public virtual exception, public virtual std::runtime_error
{
public:
    not_initialized_exception()
    : std::runtime_error("The runtime has not been initialized.")
    {
    }
};

class runtime_server : public hpx::components::component_base<runtime_server>
{
public:
    runtime_server();

    void shutdown();

    void execute(const symbol_id& func, kernel_arguments args);

    hpx::future<object> construct(type object_type, std::vector<object> arguments);
    hpx::future<void> destruct(object obj);

    distributed_access_token acquire_write_access(const object& obj);
    distributed_access_token acquire_read_access(const object& obj);

    hpx::future<hpx::id_type> get_module_library() const;

    HPX_DEFINE_COMPONENT_ACTION(runtime_server, shutdown, shutdown_action);
    HPX_DEFINE_COMPONENT_ACTION(runtime_server, execute, execute_action);
    HPX_DEFINE_COMPONENT_ACTION(runtime_server, construct, construct_action);
    HPX_DEFINE_COMPONENT_ACTION(runtime_server, destruct, destruct_action);
    HPX_DEFINE_COMPONENT_ACTION(runtime_server, acquire_write_access, acquire_write_access_action);
    HPX_DEFINE_COMPONENT_ACTION(runtime_server, acquire_read_access, acquire_read_access_action);
    HPX_DEFINE_COMPONENT_ACTION(runtime_server, get_module_library, get_module_library_action);
private:
    module_library mod_library_;
    boost::optional<virtual_address_space_wrapper::client> global_address_space_;
    global_block_pool address_block_pool_;
    std::vector<local_runtime_reference> local_runtimes_;
    boost::optional<aggregate_vpu> global_vpu_;

    dataflow_graph df_graph_;
};

class runtime : public hpx::components::client_base<runtime, runtime_server>
{
public:
    using base_type = hpx::components::client_base<runtime, runtime_server>;

    runtime() = default;

    runtime(hpx::id_type&& id);
    runtime(hpx::future<hpx::id_type>&& id);

    void shutdown();

    module_library get_module_library() const;

    [[nodiscard]] hpx::future<void> execute(const symbol_id& func, kernel_arguments args);

    hpx::future<object> construct(type object_type, std::vector<object> arguments);
    hpx::future<void> destruct(object obj);

    [[nodiscard]] hpx::future<distributed_access_token> acquire_write_access(const object& obj);
    [[nodiscard]] hpx::future<distributed_access_token> acquire_read_access(const object& obj);
    // const abi_info& abi();
};

void setup(hpx::resource::partitioner& resource_partitioner);
void init(int argc, char** argv);
void finalize();
runtime get_runtime();

std::vector<std::string> get_hpx_config();

template<typename... Args>
void execute(const symbol_id& func, Args&&... args)
{
    kernel_arguments kernel_args({std::forward<Args>(args).get_object()...});

    get_runtime().execute(func, std::move(kernel_args));
}

}

#endif
