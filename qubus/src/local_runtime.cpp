#include <hpx/config.hpp>

#include <qubus/local_runtime.hpp>

#include <qubus/logging.hpp>

#include <qubus/hpx_utils.hpp>

#include <qubus/util/get_prefix.hpp>

#include <hpx/include/lcos.hpp>

#include <boost/dll.hpp>
#include <boost/regex.hpp>

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
: service_executor_(1), global_address_space_(std::move(global_address_space_))
{
    init_logging();

    BOOST_LOG_NAMED_SCOPE("runtime");

    logger slg;

    QUBUS_LOG(slg, normal) << "Initialize the Qubus runtime";

    QUBUS_LOG(slg, normal) << "Runtime prefix: " << util::get_prefix("qubus");

    scan_for_backends();

    for (const auto& entry : backend_registry_)
    {
        if (auto backend = dynamic_cast<host_backend*>(&entry.get_backend()))
        {
            cpu_backend_ = backend;

            break;
        }
    }

    if (!cpu_backend_)
    {
        throw 0; // No valid host backend.
    }

    address_space_ = std::make_unique<local_address_space>(cpu_backend_->get_host_address_space());

    address_space_->on_page_fault([this](const object& obj) { return resolve_page_fault(obj); });

    object_factory_ = new_here<local_object_factory_server>(address_space_.get());

    QUBUS_LOG(slg, normal) << "Bootstrapping virtual multiprocessor";

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

void local_runtime::try_to_load_backend(const boost::filesystem::path& library_path)
{
    BOOST_LOG_NAMED_SCOPE("runtime");

    logger slg;

    boost::regex backend_pattern("^libqubus_([\\S]+)\\.so$");

    auto filename = library_path.filename().string();

    boost::smatch match;
    if (boost::regex_match(filename, match, backend_pattern))
    {
        auto backend_name = match[1];

        QUBUS_LOG(slg, normal) << "Loading backend '" << backend_name << "' (located at " << library_path << ")";

        boost::dll::shared_library backend_library(library_path);

        if (!backend_library)
        {
            QUBUS_LOG(slg, warning) << "Failed to load backend '" << backend_name
                                    << "' (located at " << library_path << ")";

            return;
        }

        auto init_func_name = "init_" + backend_name;

        if (!backend_library.has(init_func_name))
        {
            QUBUS_LOG(slg, warning) << "Failed to load backend '" << backend_name
                                    << "': No viable init function.";

            return;
        }

        using init_function_type = backend*(const abi_info* abi);

        auto& init_backend = backend_library.get<init_function_type>(init_func_name);

        auto backend = init_backend(&abi_info_);

        if (!backend)
        {
            QUBUS_LOG(slg, warning) << "Failed to load backend '" << backend_name
                                    << "': Backend initialization failed.";

            return;
        }

        backend_registry_.register_backend(std::move(backend_name), *backend, std::move(backend_library));
    }
}

void local_runtime::scan_for_backends()
{
    BOOST_LOG_NAMED_SCOPE("runtime");

    logger slg;

    QUBUS_LOG(slg, normal) << "Scanning for backends";

    auto backend_search_path = util::get_prefix("qubus") / "qubus/backends";

    auto first = boost::filesystem::directory_iterator(backend_search_path);
    auto last = boost::filesystem::directory_iterator();

    for (auto iter = first; iter != last; ++iter)
    {
        try_to_load_backend(iter->path());
    }
}

hpx::future<local_address_space::handle> local_runtime::resolve_page_fault(const object& obj)
{
    auto instance = global_address_space_->resolve_object(obj);

    auto page = instance.then([this, obj](hpx::future<object_instance> instance) {
        auto instance_v = instance.get();

        if (!instance_v)
            throw std::runtime_error("Page fault");

        const auto& obj_type = obj.object_type();

        auto size = obj.size();
        auto alignment = obj.alignment();

        auto page = address_space_->allocate_object_page(obj, size, alignment);

        auto& data = page.data();

        util::span<char> obj_memory(static_cast<char*>(data.ptr()), size);

        auto finished = instance_v.copy(obj_memory);

        return finished.then([page = std::move(page)](hpx::future<void> finished) {
            finished.get();

            return std::move(page);
        });
    });

    return page;
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
    if (local_qubus_runtime)
        throw 0;

    local_qubus_runtime = std::make_unique<local_runtime>(std::move(global_addr_space));

    return *local_qubus_runtime;
}

void shutdown_local_runtime()
{
    local_qubus_runtime.reset();
}

local_runtime& get_local_runtime()
{
    QUBUS_ASSERT(local_qubus_runtime, "The local runtime is not initialized.");

    return *local_qubus_runtime;
}

hpx::future<hpx::id_type>
init_local_runtime_remote(virtual_address_space_wrapper::client global_addr_space)
{
    auto copy =
        std::make_unique<virtual_address_space_wrapper::client>(std::move(global_addr_space));

    return new_here<local_runtime_reference_server>(&init_local_runtime(std::move(copy)));
}

void shutdown_local_runtime_remote()
{
    shutdown_local_runtime();
}

hpx::future<hpx::id_type> get_local_runtime_remote()
{
    return new_here<local_runtime_reference_server>(&get_local_runtime());
}

HPX_DEFINE_PLAIN_ACTION(init_local_runtime_remote, init_local_runtime_remote_action);
HPX_DEFINE_PLAIN_ACTION(shutdown_local_runtime_remote, shutdown_local_runtime_remote_action);
HPX_DEFINE_PLAIN_ACTION(get_local_runtime_remote, get_local_runtime_remote_action);

local_runtime_reference
init_local_runtime_on_locality(const hpx::id_type& locality,
                               virtual_address_space_wrapper::client superior_addr_space)
{
    return hpx::async<init_local_runtime_remote_action>(locality, std::move(superior_addr_space));
}

void shutdown_local_runtime_on_locality(const hpx::id_type& locality)
{
    hpx::async<shutdown_local_runtime_remote_action>(locality).get();
}

local_runtime_reference get_local_runtime_on_locality(const hpx::id_type& locality)
{
    return hpx::async<get_local_runtime_remote_action>(locality);
}
}

HPX_REGISTER_ACTION(qubus::init_local_runtime_remote_action,
                    QUBUS_init_local_runtime_remote_action);
HPX_REGISTER_ACTION(qubus::get_local_runtime_remote_action, QUBUS_get_local_runtime_remote_action);
