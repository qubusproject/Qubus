#include <hpx/config.hpp>

#include <qubus/local_runtime.hpp>

#include <qubus/logging.hpp>

#include <qubus/prefix.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/parallel/executors.hpp> // Workaround for missing includes.

#include <boost/dll.hpp>
#include <boost/regex.hpp>

#include <memory>
#include <utility>

using server_type = hpx::components::component<qubus::local_runtime_reference_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_local_runtime_reference_server);

typedef qubus::local_runtime_reference_server::get_local_vpu_action get_local_vpu_action;
HPX_REGISTER_ACTION(get_local_vpu_action);

namespace qubus
{

extern "C" unsigned long int cpu_backend_get_api_version();

namespace
{

std::shared_ptr<local_runtime> local_qubus_runtime;
}

local_runtime::local_runtime(std::unique_ptr<virtual_address_space> global_address_space_,
                             global_block_pool address_block_pool_, module_library mod_library_)
try : service_executor_("/qubus/service"), global_address_space_(std::move(global_address_space_))
{
    // Force the CPU backend to be linked.
    // FIXME: After we have fixed our lifetime issues, we should remove this line.
    cpu_backend_get_api_version();

    scan_for_host_backends(address_block_pool_, mod_library_);

    QUBUS_ASSERT(static_cast<bool>(backend_registry_.get_host_backend()),
                 "No host backend has been loaded.");

    auto& the_host_backend = *backend_registry_.get_host_backend();

    address_space_ =
        std::make_unique<local_address_space>(the_host_backend.get_host_address_space());

    address_space_->on_page_fault(
        [this](object_id id, local_address_space::page_fault_context ctx) mutable {
            return resolve_page_fault(id, std::move(ctx));
        });

    scan_for_vpu_backends(address_block_pool_, mod_library_);

    //local_vpu_ = std::make_unique<aggregate_vpu>(std::make_unique<round_robin_scheduler>());

    /*for (auto&& vpu : cpu_backend_->create_vpus())
    {
        local_vpu_->add_member_vpu(std::move(vpu));
    }*/

    local_vpu_ = std::move(the_host_backend.create_vpus()[0]);
}
catch (const std::exception&)
{
}

vpu& local_runtime::get_local_vpu() const
{
    return *local_vpu_;
}

local_address_space& local_runtime::get_address_space() const
{
    return *address_space_;
}

void local_runtime::try_to_load_host_backend(const boost::filesystem::path& library_path,
                                             global_block_pool address_block_pool,
                                             module_library mod_library)
{
    //BOOST_LOG_NAMED_SCOPE("runtime");

    logger slg;

    boost::regex backend_pattern("^libqubus_([\\S]+)\\.so$");

    auto filename = library_path.filename().string();

    boost::smatch match;
    if (boost::regex_match(filename, match, backend_pattern))
    {
        auto backend_name = match[1];

        //QUBUS_LOG(slg, normal) << "Loading backend '" << backend_name << "' (located at "
        //                       << library_path << ")";

        boost::dll::shared_library backend_library(library_path);

        if (!backend_library)
        {
            //QUBUS_LOG(slg, warning) << "Failed to load backend '" << backend_name
            //                        << "' (located at " << library_path << ")";

            return;
        }

        auto get_type_name = backend_name + "_get_backend_type";

        if (!backend_library.has(get_type_name))
        {
            //QUBUS_LOG(slg, warning) << "Failed to load backend '" << backend_name
            //                        << "': Missing backend type accessor.";

            return;
        }

        using get_type_type = unsigned int();

        auto& get_type = backend_library.get<get_type_type>(get_type_name);

        auto type = static_cast<backend_type>(get_type());

        if (type == backend_type::host)
        {
            auto init_func_name = "init_" + backend_name;

            if (!backend_library.has(init_func_name))
            {
                //QUBUS_LOG(slg, warning)
                //    << "Failed to load backend '" << backend_name << "': No viable init function.";

                return;
            }

            using init_function_type =
                host_backend*(const abi_info*, global_block_pool, module_library,
                              hpx::threads::executors::pool_executor*);

            auto& init_backend = backend_library.get<init_function_type>(init_func_name);

            auto backend =
                init_backend(&abi_info_, address_block_pool, mod_library, &service_executor_);

            if (!backend)
            {
                //QUBUS_LOG(slg, warning) << "Failed to load backend '" << backend_name
                //                        << "': Backend initialization failed.";

                return;
            }

            try
            {
                backend_registry_.register_host_backend(std::move(backend_name), *backend,
                                                        std::move(backend_library));
            }
            catch (const host_backend_already_set_exception& /*unused*/)
            {
                //QUBUS_LOG(slg, warning)
                //    << "Host backend has already been chosen. Ignoring new backend.";
            }
        }
    }
}

void local_runtime::try_to_load_backend(const boost::filesystem::path& library_path,
                                        global_block_pool address_block_pool,
                                        module_library mod_library)
{
    //BOOST_LOG_NAMED_SCOPE("runtime");

    //logger slg;

    boost::regex backend_pattern("^libqubus_([\\S]+)\\.so$");

    auto filename = library_path.filename().string();

    boost::smatch match;
    if (boost::regex_match(filename, match, backend_pattern))
    {
        auto backend_name = match[1];

        //QUBUS_LOG(slg, normal) << "Loading backend '" << backend_name << "' (located at "
        //                       << library_path << ")";

        boost::dll::shared_library backend_library(library_path);

        if (!backend_library)
        {
            //QUBUS_LOG(slg, warning) << "Failed to load backend '" << backend_name
            //                        << "' (located at " << library_path << ")";

            return;
        }

        auto get_type_name = backend_name + "_get_backend_type";

        if (!backend_library.has(get_type_name))
        {
            //QUBUS_LOG(slg, warning) << "Failed to load backend '" << backend_name
            //                        << "': Missing backend type accessor.";

            return;
        }

        using get_type_type = unsigned int();

        auto& get_type = backend_library.get<get_type_type>(get_type_name);

        auto type = static_cast<backend_type>(get_type());

        if (type == backend_type::vpu)
        {
            auto init_func_name = "init_" + backend_name;

            if (!backend_library.has(init_func_name))
            {
                //QUBUS_LOG(slg, warning)
                //    << "Failed to load backend '" << backend_name << "': No viable init function.";

                return;
            }

            using init_function_type =
                backend*(const abi_info*, local_address_space*, global_block_pool, module_library,
                         hpx::threads::executors::pool_executor*);

            auto& init_backend = backend_library.get<init_function_type>(init_func_name);

            QUBUS_ASSERT(address_space_, "Invalid address space");

            auto backend = init_backend(&abi_info_, address_space_.get(), address_block_pool,
                                        mod_library, &service_executor_);

            if (!backend)
            {
                //QUBUS_LOG(slg, warning) << "Failed to load backend '" << backend_name
                //                        << "': Backend initialization failed.";

                return;
            }

            backend_registry_.register_backend(std::move(backend_name), *backend,
                                               std::move(backend_library));
        }
    }
}

void local_runtime::scan_for_host_backends(global_block_pool address_block_pool,
                                           module_library mod_library)
{
    {
        //BOOST_LOG_NAMED_SCOPE("runtime");

        //logger slg;

        //QUBUS_LOG(slg, normal) << "Scanning for host backends";
    }

    auto backend_search_path = get_prefix() / "qubus/backends";

    {
        auto first = boost::filesystem::directory_iterator(backend_search_path);
        auto last = boost::filesystem::directory_iterator();

        for (auto iter = first; iter != last; ++iter)
        {
            try_to_load_host_backend(iter->path(), address_block_pool, mod_library);
        }
    }

    host_backend* the_host_backend;

    if (auto host_backend = backend_registry_.get_host_backend())
    {
        the_host_backend = &host_backend.get();
    }
    else
    {
        the_host_backend = nullptr;
    }

    if (!the_host_backend)
    {
        throw 0; // No valid host backend.
    }
}

void local_runtime::scan_for_vpu_backends(global_block_pool address_block_pool,
                                          module_library mod_library)
{
    {
        //BOOST_LOG_NAMED_SCOPE("runtime");

        //logger slg;

        //QUBUS_LOG(slg, normal) << "Scanning for VPU backends";
    }

    auto backend_search_path = get_prefix() / "qubus/backends";

    {
        auto first = boost::filesystem::directory_iterator(backend_search_path);
        auto last = boost::filesystem::directory_iterator();

        for (auto iter = first; iter != last; ++iter)
        {
            try_to_load_backend(iter->path(), address_block_pool, mod_library);
        }
    }
}

hpx::future<local_address_space::object_instance_type>
local_runtime::resolve_page_fault(object_id id, local_address_space::page_fault_context ctx)
{
    auto token = global_address_space_->resolve_object(id);

    auto instance = token.then(get_local_runtime().get_service_executor(),
                               [this, id, ctx](hpx::future<instance_token> token) mutable {
                                   auto token_v = token.get();

                                   if (!token_v)
                                       throw std::runtime_error("Page fault");

                                   host_allocator* alloc;

                                   auto instance =
                                       token_v.acquire_instance(get_service_executor(), *alloc, 1);

                                   return instance.get();
                               });

    return instance;
}

local_runtime_reference_server::local_runtime_reference_server(
    std::weak_ptr<local_runtime> runtime_)
: runtime_(runtime_)
{
}

std::unique_ptr<remote_vpu_reference> local_runtime_reference_server::get_local_vpu() const
{
    auto runtime = runtime_.lock();

    QUBUS_ASSERT(runtime, "The referenced local runtime is invalid.");

    return std::make_unique<remote_vpu_reference>(
        hpx::local_new<remote_vpu_reference>(&runtime->get_local_vpu()));
}

local_runtime_reference::local_runtime_reference(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

std::unique_ptr<remote_vpu_reference> local_runtime_reference::get_local_vpu() const
{
    return hpx::async<local_runtime_reference_server::get_local_vpu_action>(this->get_id()).get();
}

std::weak_ptr<local_runtime>
init_local_runtime(std::unique_ptr<virtual_address_space> global_addr_space,
                   global_block_pool address_block_pool, module_library mod_library)
{
    if (local_qubus_runtime)
        throw 0;

    local_qubus_runtime = std::make_shared<local_runtime>(
        std::move(global_addr_space), std::move(address_block_pool), std::move(mod_library));

    return local_qubus_runtime;
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
init_local_runtime_remote(virtual_address_space_wrapper::client global_addr_space,
                          global_block_pool address_block_pool, module_library mod_library)
{
    auto copy =
        std::make_unique<virtual_address_space_wrapper::client>(std::move(global_addr_space));

    return hpx::local_new<local_runtime_reference_server>(
        init_local_runtime(std::move(copy), std::move(address_block_pool), std::move(mod_library)));
}

void shutdown_local_runtime_remote()
{
    shutdown_local_runtime();
}

hpx::future<hpx::id_type> get_local_runtime_remote()
{
    return hpx::local_new<local_runtime_reference_server>(local_qubus_runtime);
}

HPX_DEFINE_PLAIN_ACTION(init_local_runtime_remote, init_local_runtime_remote_action);
HPX_DEFINE_PLAIN_ACTION(shutdown_local_runtime_remote, shutdown_local_runtime_remote_action);
HPX_DEFINE_PLAIN_ACTION(get_local_runtime_remote, get_local_runtime_remote_action);

local_runtime_reference
init_local_runtime_on_locality(const hpx::id_type& locality,
                               virtual_address_space_wrapper::client superior_addr_space,
                               global_block_pool address_block_pool, module_library mod_library)
{
    // FIXME: Reevaluate the impact of the manual unwrapping of the future.
    return hpx::async<init_local_runtime_remote_action>(locality, std::move(superior_addr_space),
                                                        std::move(address_block_pool),
                                                        std::move(mod_library))
        .get();
}

void shutdown_local_runtime_on_locality(const hpx::id_type& locality)
{
    hpx::async<shutdown_local_runtime_remote_action>(locality).get();
}

local_runtime_reference get_local_runtime_on_locality(const hpx::id_type& locality)
{
    // FIXME: Reevaluate the impact of the manual unwrapping of the future.
    return hpx::async<get_local_runtime_remote_action>(locality).get();
}
} // namespace qubus

HPX_REGISTER_ACTION(qubus::init_local_runtime_remote_action,
                    QUBUS_init_local_runtime_remote_action);
HPX_REGISTER_ACTION(qubus::get_local_runtime_remote_action, QUBUS_get_local_runtime_remote_action);
