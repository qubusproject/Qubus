#include <hpx/config.hpp>

#include <qbb/qubus/local_runtime.hpp>

#include <qbb/qubus/aggregate_vpu.hpp>

#include <qbb/qubus/logging.hpp>

#include <qbb/qubus/hpx_utils.hpp>

#include <qbb/util/get_prefix.hpp>

#include <hpx/include/lcos.hpp>

#include <memory>
#include <utility>

using server_type = hpx::components::component<qbb::qubus::local_runtime_server>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_local_runtime_server);

HPX_REGISTER_ACTION(qbb::qubus::init_local_runtime_action, qbb_qubus_init_local_runtime_action);
HPX_REGISTER_ACTION(qbb::qubus::get_local_runtime_action, qbb_qubus_get_local_runtime);

typedef qbb::qubus::local_runtime_server::get_local_object_factory_action get_local_object_factory_action;
HPX_REGISTER_ACTION_DECLARATION(get_local_object_factory_action);
HPX_REGISTER_ACTION(get_local_object_factory_action)

typedef qbb::qubus::local_runtime_server::get_local_vpu_action get_local_vpu_action;
HPX_REGISTER_ACTION_DECLARATION(get_local_vpu_action);
HPX_REGISTER_ACTION(get_local_vpu_action);

namespace qbb
{
namespace qubus
{

namespace
{

local_runtime local_qubus_runtime;

}

extern "C" backend* init_cpu_backend(const abi_info*);

local_runtime_server::local_runtime_server()
//: cpu_plugin_(util::get_prefix("qubus") / "qubus/backends/libqubus_cpu_backend.so")
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

    using local_object_factory_component_type = hpx::components::component<local_object_factory_server>;
    object_factory_ = hpx::make_ready_future(
        hpx::id_type(hpx::components::server::construct<local_object_factory_component_type>(
                         address_space_.get()),
                     hpx::id_type::managed));

    local_vpu_ = new_here<aggregate_vpu>();

    auto local_cpu_ptr = hpx::get_ptr_sync<aggregate_vpu>(local_vpu_.get_id());

    for (const auto& vpu : cpu_backend_->vpus())
    {
        local_cpu_ptr->add_member_vpu(vpu);
    }
}

local_runtime_server::~local_runtime_server()
{
}

local_object_factory local_runtime_server::get_local_object_factory() const
{
    return object_factory_;
}

vpu local_runtime_server::get_local_vpu() const
{
    return local_vpu_;
}

local_runtime::local_runtime(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

local_object_factory local_runtime::get_local_object_factory() const
{
    return hpx::async<get_local_object_factory_action>(this->get_id()).get();
}

vpu local_runtime::get_local_vpu() const
{
    return hpx::async<get_local_vpu_action>(this->get_id()).get();
}

local_runtime init_local_runtime()
{
    local_qubus_runtime = new_here<local_runtime_server>();

    return local_qubus_runtime;
}

local_runtime get_local_runtime()
{
    return local_qubus_runtime;
}

}
}


