#include <hpx/config.hpp>

#include <qubus/backend.hpp>
#include <qubus/host_backend.hpp>

#include <qubus/abi_info.hpp>
#include <qubus/local_address_space.hpp>
#include <qubus/block_address_allocator.hpp>
#include <qubus/module_library.hpp>
#include <qubus/performance_models/unified_performance_model.hpp>

#include <qubus/host_memory.hpp>

#include <qubus/backends/cpu/cpu_allocator.hpp>
#include <qubus/backends/cpu/cpu_compiler.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/IR/type_inference.hpp>

#include <qubus/util/make_unique.hpp>

#include <qubus/qubus_export.h>

#include <hpx/async.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/threads.hpp>

#include <boost/optional.hpp>
#include <boost/signals2.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/make_unique.hpp>
#include <qubus/util/optional_ref.hpp>
#include <qubus/util/unused.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <thread>

namespace qubus
{

class cpu_runtime
{
public:
    cpu_runtime() : scratch_mem_(8 * 1024 * 1024), current_stack_ptr_(scratch_mem_.data())
    {
    }

    void* alloc_scratch_mem(util::index_t size)
    {
        void* addr = current_stack_ptr_;

        current_stack_ptr_ += size;

        return addr;
    }

    void dealloc_scratch_mem(util::index_t size)
    {
        current_stack_ptr_ -= size;
    }

private:
    std::vector<char> scratch_mem_;
    char* current_stack_ptr_;
};

extern "C" QUBUS_EXPORT void* QUBUS_cpurt_alloc_scatch_mem(cpu_runtime* runtime, util::index_t size)
{
    return runtime->alloc_scratch_mem(size);
}

extern "C" QUBUS_EXPORT void QUBUS_cpurt_dealloc_scratch_mem(cpu_runtime* runtime,
                                                             util::index_t size)
{
    runtime->dealloc_scratch_mem(size);
}

class caching_cpu_comiler
{
public:
    explicit caching_cpu_comiler(module_library mod_library_)
    : mod_library_(std::move(mod_library_))
    {
    }

    const cpu_plan& compile(const symbol_id& func) const
    {
        auto module_id = func.get_prefix();

        std::unique_lock<hpx::lcos::local::mutex> guard(cache_mutex_);

        auto pos = compilation_cache_.find(module_id);

        if (pos != compilation_cache_.end())
        {
            return *pos->second;
        }
        else
        {
            hpx::threads::executors::default_executor executor(hpx::threads::thread_stacksize_huge);

            auto compilation =
                hpx::async(executor,
                           [module_id, this] {
                               auto code = mod_library_.lookup(module_id).get();

                               return underlying_compiler_.compile_computelet(std::move(code));
                           })
                    .get();

            pos = compilation_cache_.emplace(module_id, std::move(compilation)).first;

            return *pos->second;
        }
    }

    std::function<void(void*, const std::vector<void*>&, void*)>
    get_constructor(const type& datatype)
    {
        return underlying_compiler_.get_constructor(datatype);
    }

private:
    module_library mod_library_;
    mutable cpu_compiler underlying_compiler_; // FIXME: Make the cpmpiler non-mutable.
    mutable std::unordered_map<symbol_id, std::unique_ptr<cpu_plan>> compilation_cache_;
    mutable hpx::lcos::local::mutex cache_mutex_;
};

class cpu_vpu : public vpu
{
public:
    cpu_vpu(host_address_space& address_space_, global_block_pool address_block_pool_, abi_info abi_, module_library mod_library_)
    : compiler_(mod_library_),
      address_space_(&address_space_),
      address_allocator_(std::move(address_block_pool_)),
      abi_(std::move(abi_)),
      perf_model_(mod_library_, address_space_)
    {
    }

    virtual ~cpu_vpu() = default;

    [[nodiscard]] hpx::future<void> execute(const symbol_id& func, execution_context ctx) override
    {
        const auto& compilation = compiler_.compile(func);

        hpx::future<void> task_done = hpx::async([this, &compilation, func, ctx]() mutable {
            auto task_start = std::chrono::steady_clock::now();

            std::vector<void*> task_args;

            std::vector<host_address_space::handle> pages;

            for (const auto& arg : ctx.args())
            {
                auto page = address_space_->resolve_object(arg.id()).get();

                task_args.push_back(page.data().ptr());

                pages.push_back(std::move(page));
            }

            for (const auto& result : ctx.results())
            {
                auto page = address_space_->resolve_object(result.id()).get();

                task_args.push_back(page.data().ptr());

                pages.push_back(std::move(page));
            }

            cpu_runtime runtime;

            compilation.execute(func, task_args, runtime);

            auto task_end = std::chrono::steady_clock::now();

            auto task_duration =
                std::chrono::duration_cast<std::chrono::microseconds>(task_end - task_start);

            perf_model_.sample_execution_time(func, ctx, std::move(task_duration));
        });

        return task_done;
    }

    [[nodiscard]] hpx::future<boost::optional<performance_estimate>>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const override
    {
        auto estimate = perf_model_.try_estimate_execution_time(func, ctx);

        return hpx::make_ready_future(std::move(estimate));
    }

    [[nodiscard]] hpx::future<object> construct_local_object(type object_type,
                                                             std::vector<object> arguments) override
    {
        auto constructor = compiler_.get_constructor(object_type);

        std::vector<void*> args;

        std::vector<host_address_space::handle> pages;

        for (const auto& arg : arguments)
        {
            auto page = address_space_->resolve_object(arg.id()).get();

            args.push_back(page.data().ptr());

            pages.push_back(std::move(page));
        }

        object_id id = address_allocator_.acquire_new_id();

        address_space_->allocate_page(id, object_type, 0,
                                      [constructor = std::move(constructor), args = std::move(args),
                                       pages = std::move(pages)](host_object_instance& instance) {
                                          cpu_runtime runtime;

                                          constructor(instance.data().ptr(), args, &runtime);

                                          return hpx::make_ready_future();
                                      });

        return hpx::make_ready_future(object(id, object_type));
    }

private:
    caching_cpu_comiler compiler_;
    host_address_space* address_space_;
    block_address_allocator address_allocator_;
    abi_info abi_;

    unified_performance_model perf_model_;
};

class cpu_backend final : public host_backend
{
public:
    cpu_backend(const abi_info& abi_, global_block_pool address_block_pool_, module_library mod_library_,
                hpx::threads::executors::pool_executor& service_executor_)
    : abi_(&abi_),
      address_space_(std::make_unique<host_address_space>(host_allocator(), service_executor_)),
      address_block_pool_(std::move(address_block_pool_)),
      mod_library_(std::move(mod_library_))
    {
        // use hwloc to obtain informations over all local CPUs
    }

    virtual ~cpu_backend() = default;

    std::string id() const override
    {
        return "qubus.cpu";
    }

    std::vector<std::unique_ptr<vpu>> create_vpus() const override
    {
        std::vector<std::unique_ptr<vpu>> vpus;

        unsigned int concurrency = std::thread::hardware_concurrency();

        for (unsigned int i = 0; i < concurrency; ++i)
        {
            vpus.push_back(std::make_unique<cpu_vpu>(*address_space_, address_block_pool_, *abi_, mod_library_));
        }

        return vpus;
    }

    host_address_space& get_host_address_space() override
    {
        return *address_space_;
    }


private:
    const abi_info* abi_;
    std::unique_ptr<host_address_space> address_space_;
    global_block_pool address_block_pool_;
    module_library mod_library_;
};

extern "C" QUBUS_EXPORT unsigned int cpu_backend_get_backend_type()
{
    return static_cast<unsigned int>(backend_type::host);
}

extern "C" QUBUS_EXPORT unsigned long int cpu_backend_get_api_version()
{
    return 0;
}

std::unique_ptr<cpu_backend> the_cpu_backend;
std::once_flag cpu_backend_init_flag;

extern "C" QUBUS_EXPORT host_backend*
init_cpu_backend(const abi_info* abi, global_block_pool address_block_pool, module_library mod_library,
                 hpx::threads::executors::pool_executor* service_executor)
{
    std::call_once(cpu_backend_init_flag, [&] {
        the_cpu_backend =
            util::make_unique<cpu_backend>(*abi, std::move(address_block_pool), std::move(mod_library), *service_executor);
    });

    return the_cpu_backend.get();
}
} // namespace qubus
