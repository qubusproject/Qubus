#include <hpx/config.hpp>

#include <qubus/local_runtime.hpp>

#include <qubus/logging.hpp>

#include <qubus/hpx_utils.hpp>

#include <qubus/util/get_prefix.hpp>

#include <hpx/include/lcos.hpp>

#include <memory>
#include <utility>

using server_type = hpx::components::component<qubus::local_runtime_reference_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_local_runtime_reference_server);

typedef qubus::local_runtime_reference_server::get_local_object_factory_action
    get_local_object_factory_action;
HPX_REGISTER_ACTION_DECLARATION(get_local_object_factory_action);
HPX_REGISTER_ACTION(get_local_object_factory_action)

typedef qubus::local_runtime_reference_server::get_local_vpu_action get_local_vpu_action;
HPX_REGISTER_ACTION_DECLARATION(get_local_vpu_action);
HPX_REGISTER_ACTION(get_local_vpu_action);

namespace qubus
{

namespace
{

std::unique_ptr<local_runtime> local_qubus_runtime;
}

extern "C" backend* init_cpu_backend(const abi_info*);

local_runtime::local_runtime(std::unique_ptr<virtual_address_space> global_address_space_)
//: cpu_plugin_(util::get_prefix("qubus") / "qubus/backends/libqubus_cpu_backend.so")
: global_address_space_(std::move(global_address_space_))
{
    init_logging();

    BOOST_LOG_NAMED_SCOPE("runtime");

    logger slg;

    QUBUS_LOG(slg, normal) << "Initialize the Qubus runtime";

    QUBUS_LOG(slg, normal) << "Runtime prefix: " << util::get_prefix("qubus");

    QUBUS_LOG(slg, normal) << "Bootstrapping virtual multiprocessor";

    QUBUS_LOG(slg, normal) << "Scanning for backends";

    QUBUS_LOG(slg, normal) << "Loading backend 'cpu_backend'";

    //auto init_cpu_backend = cpu_plugin_.get<backend*(const abi_info*)>("init_cpu_backend");

    cpu_backend_ = dynamic_cast<host_backend*>(init_cpu_backend(&abi_info_));

    if (!cpu_backend_)
    {
        throw 0;
    }

    address_space_ = std::make_unique<local_address_space>(cpu_backend_->get_host_address_space());

    object_factory_ = new_here<local_object_factory_server>(address_space_.get());

    //local_vpu_ = std::make_unique<aggregate_vpu>(std::make_unique<round_robin_scheduler>());

    /*for (auto&& vpu : cpu_backend_->create_vpus())
    {
        local_vpu_->add_member_vpu(std::move(vpu));
    }*/

    local_vpu_ = std::move(cpu_backend_->create_vpus()[0]);
}

local_object_factory local_runtime::get_local_object_factory() const
{
    return object_factory_;
}

vpu& local_runtime::get_local_vpu() const
{
    return *local_vpu_;
}

local_address_space& local_runtime::get_address_space() const
{
    return *address_space_;
}

local_runtime_reference_server::local_runtime_reference_server(local_runtime* runtime_)
: runtime_(runtime_)
{
}

hpx::future<hpx::id_type> local_runtime_reference_server::get_local_object_factory() const
{
    return hpx::make_ready_future(runtime_->get_local_object_factory().get());
}

std::unique_ptr<remote_vpu_reference> local_runtime_reference_server::get_local_vpu() const
{
    return std::make_unique<remote_vpu_reference>(
        new_here<remote_vpu_reference_server>(&runtime_->get_local_vpu()));
}

local_runtime_reference::local_runtime_reference(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

local_object_factory local_runtime_reference::get_local_object_factory() const
{
    return hpx::async<local_runtime_reference_server::get_local_object_factory_action>(
        this->get_id());
}

std::unique_ptr<remote_vpu_reference> local_runtime_reference::get_local_vpu() const
{
    return hpx::async<local_runtime_reference_server::get_local_vpu_action>(this->get_id()).get();
}

local_runtime& init_local_runtime(std::unique_ptr<virtual_address_space> global_addr_space)
{
    local_qubus_runtime = std::make_unique<local_runtime>(std::move(global_addr_space));

    return *local_qubus_runtime;
}

local_runtime& get_local_runtime()
{
    return *local_qubus_runtime;
}

hpx::future<hpx::id_type> init_local_runtime_remote(virtual_address_space_wrapper::client global_addr_space)
{
    auto copy = std::make_unique<virtual_address_space_wrapper::client>(std::move(global_addr_space));

    return new_here<local_runtime_reference_server>(&init_local_runtime(std::move(copy)));
}

hpx::future<hpx::id_type> get_local_runtime_remote()
{
    return new_here<local_runtime_reference_server>(&get_local_runtime());
}

HPX_DEFINE_PLAIN_ACTION(init_local_runtime_remote, init_local_runtime_remote_action);
HPX_DEFINE_PLAIN_ACTION(get_local_runtime_remote, get_local_runtime_remote_action);

local_runtime_reference
init_local_runtime_on_locality(const hpx::id_type& locality,
                               virtual_address_space_wrapper::client superior_addr_space)
{
    return hpx::async<init_local_runtime_remote_action>(locality, std::move(superior_addr_space));
}

local_runtime_reference get_local_runtime_on_locality(const hpx::id_type& locality)
{
    return hpx::async<get_local_runtime_remote_action>(locality);
}
}

HPX_REGISTER_ACTION(qubus::init_local_runtime_remote_action,
                    QUBUS_init_local_runtime_remote_action);
HPX_REGISTER_ACTION(qubus::get_local_runtime_remote_action, QUBUS_get_local_runtime_remote_action);
