#include <qubus/runtime.hpp>

#include <qubus/basic_address_space.hpp>
#include <qubus/logging.hpp>
#include <qubus/prefix.hpp>
#include <qubus/scheduling/uniform_fill_scheduler.hpp>

#include <hpx/parallel/executors.hpp> // Workaround for missing includes.

#include <qubus/util/unused.hpp>

using server_type = hpx::components::component<qubus::runtime_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_runtime_server);

typedef qubus::runtime_server::shutdown_action shutdown_action;
HPX_REGISTER_ACTION(shutdown_action, qubus_runtime_server_shutdown_action);

typedef qubus::runtime_server::execute_action execute_action;
HPX_REGISTER_ACTION(execute_action, qubus_runtime_server_execute_action);

typedef qubus::runtime_server::construct_action construct_action;
HPX_REGISTER_ACTION(construct_action, qubus_runtime_server_construct_action);

typedef qubus::runtime_server::destruct_action destruct_action;
HPX_REGISTER_ACTION(destruct_action, qubus_runtime_server_destruct_action);

using acquire_write_access_action = qubus::runtime_server::acquire_write_access_action;
HPX_REGISTER_ACTION(acquire_write_access_action, qubus_runtime_acquire_write_access_action);

using acquire_read_access_action = qubus::runtime_server::acquire_read_access_action;
HPX_REGISTER_ACTION(acquire_read_access_action, qubus_runtime_acquire_read_access_action);

typedef qubus::runtime_server::get_module_library_action get_module_library_action;
HPX_REGISTER_ACTION(get_module_library_action, qubus_runtime_server_get_module_library_action);

namespace qubus
{

runtime_server::runtime_server()
: mod_library_(hpx::local_new<module_library>()),
  address_block_pool_(hpx::local_new<global_block_pool>())
{
    init_logging();

    /*{
        BOOST_LOG_NAMED_SCOPE("runtime");

        logger slg;

        QUBUS_LOG(slg, normal) << "Initialize the Qubus runtime";

        QUBUS_LOG(slg, normal) << "Runtime prefix: " << get_prefix();
    }*/

    auto addr_space_impl = std::make_unique<basic_address_space>();

    global_address_space_ =
        hpx::local_new<virtual_address_space_wrapper>(std::move(addr_space_impl));

    QUBUS_ASSERT(static_cast<bool>(global_address_space_),
                 "This subsystem is not properly initialized.");

    for (const auto& locality : hpx::find_all_localities())
    {
        local_runtimes_.push_back(init_local_runtime_on_locality(
            locality, *global_address_space_, address_block_pool_, mod_library_));
    }

    /*{
        BOOST_LOG_NAMED_SCOPE("runtime");

        logger slg;

        QUBUS_LOG(slg, normal) << "Bootstrapping virtual multiprocessor";
    }*/

    global_vpu_ = aggregate_vpu(std::make_unique<uniform_fill_scheduler>());

    for (const auto& runtime : local_runtimes_)
    {
        global_vpu_->add_member_vpu(runtime.get_local_vpu());
    }

    hpx::wait_all(local_runtimes_);
}

void runtime_server::shutdown()
{
    df_graph_.finalize();

    mod_library_.free();

    global_vpu_.reset();

    for (const auto& locality : hpx::find_all_localities())
    {
        shutdown_local_runtime_on_locality(locality);
    }

    global_address_space_.reset();

    finalize_logging();
}

void runtime_server::execute(const symbol_id& func, kernel_arguments kernel_args)
{
    /*std::vector<token> tokens;

    std::vector<object> args;
    std::vector<object> results;

    std::vector<distributed_future<void>> dependencies;

    for (auto& arg : kernel_args.args())
    {
        auto token = arg.acquire_read_access();

        auto is_valid = token.when_valid();

        dependencies.push_back(std::move(is_valid));

        args.push_back(arg);

        tokens.push_back(std::move(token));
    }

    for (auto& result : kernel_args.results())
    {
        auto token = result.acquire_write_access();

        auto is_valid = token.when_valid();

        dependencies.push_back(std::move(is_valid));

        results.push_back(result);

        tokens.push_back(std::move(token));
    }

    hpx::future<void> dependencies_ready = hpx::when_all(std::move(dependencies));

    execution_context ctx(std::move(args), std::move(results));

    hpx::future<void> is_finished = dependencies_ready.then(
        get_local_runtime().get_service_executor(),
        [ this, func, ctx = std::move(ctx) ](hpx::future<void> dependencies_ready) {
            dependencies_ready.get();

            return global_vpu_->execute(func, std::move(ctx));
        });

    is_finished.then([tokens = std::move(tokens)](hpx::future<void> is_finished) mutable {
        is_finished.get();

        for (auto& token : tokens)
        {
            token.release();
        }
    });*/

    // TODO: Make sure that all tasks are run.

    std::vector<object> args;
    std::vector<object> results;

    std::vector<hpx::future<access_token>> dependencies;

    for (auto& arg : kernel_args.args())
    {
        auto token = df_graph_.schedule_read(arg);

        dependencies.push_back(std::move(token));

        args.push_back(arg);
    }

    for (auto& result : kernel_args.results())
    {
        auto token = df_graph_.schedule_modification(result);

        dependencies.push_back(std::move(token));

        results.push_back(result);
    }

    hpx::future<std::vector<hpx::future<access_token>>> dependencies_ready =
        hpx::when_all(std::move(dependencies));

    execution_context ctx(std::move(args), std::move(results));

    dependencies_ready.then(
        get_local_runtime().get_service_executor(),
        [this, func, ctx = std::move(ctx)](
            hpx::future<std::vector<hpx::future<access_token>>> dependencies_ready) {
            auto tokens = dependencies_ready.get();

            global_vpu_->execute(func, std::move(ctx)).get();
        });
}

hpx::future<object> runtime_server::construct(type object_type, std::vector<object> arguments)
{
    std::vector<hpx::future<access_token>> dependencies;

    for (auto& arg : arguments)
    {
        auto token = df_graph_.schedule_read(arg);

        dependencies.push_back(std::move(token));
    }

    hpx::future<std::vector<hpx::future<access_token>>> dependencies_ready =
        hpx::when_all(std::move(dependencies));

    return dependencies_ready.then(
        get_local_runtime().get_service_executor(),
        [this, object_type = std::move(object_type),
         arguments = std::move(arguments)](auto dependencies_ready) mutable {
            auto tokens = dependencies_ready.get();

            return global_vpu_->construct_local_object(std::move(object_type), std::move(arguments))
                .get();
        });
}

hpx::future<void> runtime_server::destruct(object obj)
{
    return global_address_space_->free_object(obj.id());
}

distributed_access_token runtime_server::acquire_write_access(const object& obj)
{
    return df_graph_.schedule_modification(obj).get();
}

distributed_access_token runtime_server::acquire_read_access(const object& obj)
{
    return df_graph_.schedule_read(obj).get();
}

hpx::future<hpx::id_type> runtime_server::get_module_library() const
{
    return hpx::make_ready_future(mod_library_.get());
}

runtime::runtime(hpx::id_type&& id) : base_type(std::move(id))
{
}

runtime::runtime(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

void runtime::shutdown()
{
    hpx::async<runtime_server::shutdown_action>(this->get_id()).get();
}

hpx::future<void> runtime::execute(const symbol_id& func, kernel_arguments args)
{
    return hpx::async<runtime_server::execute_action>(this->get_id(), func, std::move(args));
}

hpx::future<object> runtime::construct(type object_type, std::vector<object> arguments)
{
    return hpx::async<runtime_server::construct_action>(this->get_id(), std::move(object_type),
                                                        std::move(arguments));
}

hpx::future<void> runtime::destruct(object obj)
{
    return hpx::async<runtime_server::destruct_action>(this->get_id(), std::move(obj));
}

hpx::future<distributed_access_token> runtime::acquire_write_access(const object& obj)
{
    return hpx::async<runtime_server::acquire_write_access_action>(this->get_id(), obj);
}

hpx::future<distributed_access_token> runtime::acquire_read_access(const object& obj)
{
    return hpx::async<runtime_server::acquire_read_access_action>(this->get_id(), obj);
}

module_library runtime::get_module_library() const
{
    // FIXME: Reevaluate the impact of the manual unwrapping of the future.
    return hpx::async<runtime_server::get_module_library_action>(this->get_id()).get();
}

void setup(hpx::resource::partitioner& resource_partitioner)
{
    resource_partitioner.create_thread_pool("/qubus/service",
                                            hpx::resource::scheduling_policy::local_priority_fifo);

    resource_partitioner.add_resource(resource_partitioner.numa_domains()[0].cores()[0].pus()[0],
                                      "/qubus/service");
}

namespace
{
runtime global_runtime;
}

void init(int QUBUS_UNUSED(argc), char** QUBUS_UNUSED(argv))
{
    if (!global_runtime)
    {
        global_runtime = hpx::new_<runtime>(hpx::find_here());
        hpx::agas::register_name(hpx::launch::sync, "/qubus/runtime", global_runtime.get_id());
    }
}

void finalize()
{
    auto rt = global_runtime;
    global_runtime.free();

    hpx::agas::unregister_name("/qubus/runtime");

    rt.shutdown();
}

runtime get_runtime()
{
    auto rt = hpx::agas::resolve_name(hpx::launch::sync, "/qubus/runtime");

    if (!rt)
        throw not_initialized_exception();

    return rt;
}

std::vector<std::string> get_hpx_config()
{
    std::vector<std::string> cfg = {"hpx.commandline.aliasing=0",
                                    "hpx.commandline.allow_unknown=1"};

    return cfg;
}
} // namespace qubus
