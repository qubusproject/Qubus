#ifndef QUBUS_RUNTIME_HPP
#define QUBUS_RUNTIME_HPP

#include <hpx/config.hpp>

#include <qubus/local_runtime.hpp>

#include <qubus/aggregate_vpu.hpp>
#include <qubus/virtual_address_space.hpp>

#include <qubus/computelet.hpp>
#include <qubus/kernel_arguments.hpp>

#include <qubus/object_factory.hpp>

#include <boost/optional.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/actions.hpp>

#include <string>
#include <vector>

namespace qubus
{

class runtime_server : public hpx::components::component_base<runtime_server>
{
public:
    runtime_server();

    void shutdown();

    void execute(computelet c, kernel_arguments args);

    hpx::future<hpx::id_type> get_object_factory() const;

    HPX_DEFINE_COMPONENT_ACTION(runtime_server, shutdown, shutdown_action);
    HPX_DEFINE_COMPONENT_ACTION(runtime_server, execute, execute_action);
    HPX_DEFINE_COMPONENT_ACTION(runtime_server, get_object_factory, get_object_factory_action);
private:
    boost::optional<virtual_address_space_wrapper::client> global_address_space_;
    std::vector<local_runtime_reference> local_runtimes_;
    object_factory obj_factory_;
    boost::optional<aggregate_vpu> global_vpu_;
};

class runtime : public hpx::components::client_base<runtime, runtime_server>
{
public:
    using base_type = hpx::components::client_base<runtime, runtime_server>;

    runtime() = default;

    runtime(hpx::future<hpx::id_type>&& id);

    void shutdown();

    object_factory get_object_factory() const;

    hpx::future<void> execute(computelet c, kernel_arguments args);

    // const abi_info& abi();
};

void init(int argc, char** argv);
void finalize();
runtime get_runtime();

std::vector<std::string> get_hpx_config();

template<typename... Args>
void execute(const computelet& c, Args&&... args)
{
    kernel_arguments kernel_args({std::forward<Args>(args).get_object()...});

    get_runtime().execute(c, std::move(kernel_args));
}

}

#endif
