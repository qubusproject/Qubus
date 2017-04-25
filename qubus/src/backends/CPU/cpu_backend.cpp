#include <hpx/config.hpp>

#include <qubus/backend.hpp>
#include <qubus/host_backend.hpp>

#include <qubus/abi_info.hpp>
#include <qubus/local_address_space.hpp>
#include <qubus/performance_models/unified_performance_model.hpp>

#include <qubus/host_allocator.hpp>

#include <qubus/backends/cpu/cpu_compiler.hpp>
#include <qubus/backends/cpu/cpu_allocator.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/IR/type_inference.hpp>

#include <qubus/util/make_unique.hpp>

#include <qubus/qubus_export.h>

#include <hpx/async.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/threads.hpp>

#include <qubus/hpx_utils.hpp>

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

extern "C" QUBUS_EXPORT void* QUBUS_cpurt_alloc_scatch_mem(cpu_runtime* runtime,
                                                                   util::index_t size)
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
    const cpu_plan& compile(const computelet& c) const
    {
        std::unique_lock<hpx::lcos::local::mutex> guard(cache_mutex_);

        auto pos = compilation_cache_.find(c.get_id().get_gid());

        if (pos != compilation_cache_.end())
        {
            return *pos->second;
        }
        else
        {
            hpx::threads::executors::default_executor executor(hpx::threads::thread_stacksize_huge);

            auto compilation = hpx::async(executor, [&c, this] {
                                   return underlying_compiler_.compile_computelet(c.code().get());
                               }).get();

            pos = compilation_cache_.emplace(c.get_id().get_gid(), std::move(compilation)).first;

            return *pos->second;
        }
    }

private:
    mutable cpu_compiler underlying_compiler_; // FIXME: Make the cpmpiler non-mutable.
    mutable std::unordered_map<hpx::naming::gid_type, std::unique_ptr<cpu_plan>> compilation_cache_;
    mutable hpx::lcos::local::mutex cache_mutex_;
};

class cpu_vpu : public vpu
{
public:
    cpu_vpu(host_address_space& address_space_, abi_info abi_)
    : address_space_(&address_space_), abi_(std::move(abi_))
    {
    }

    virtual ~cpu_vpu() = default;

    hpx::future<void> execute(computelet c, execution_context ctx) override
    {
        const auto& compilation = compiler_.compile(c);

        auto deps_ready = ctx.when_ready();

        hpx::future<void> task_done = hpx::dataflow(
            [this, &compilation, c, ctx](hpx::future<void> deps_ready) mutable {
                deps_ready.get();

                auto task_start = std::chrono::steady_clock::now();

                std::vector<void*> task_args;

                std::vector<host_address_space::handle> pages;

                for (const auto& arg : ctx.args())
                {
                    auto page = address_space_->resolve_object(arg).get();

                    task_args.push_back(page.data().ptr());

                    pages.push_back(std::move(page));
                }

                for (const auto& result : ctx.results())
                {
                    auto page = address_space_->resolve_object(result).get();

                    task_args.push_back(page.data().ptr());

                    pages.push_back(std::move(page));
                }

                cpu_runtime runtime;

                compilation.execute(task_args, runtime);

                auto task_end = std::chrono::steady_clock::now();

                auto task_duration =
                    std::chrono::duration_cast<std::chrono::microseconds>(task_end - task_start);

                perf_model_.sample_execution_time(c, ctx, std::move(task_duration));
            },
            std::move(deps_ready));

        return task_done;
    }

    hpx::future<boost::optional<performance_estimate>>
    try_estimate_execution_time(const computelet& c, const execution_context& ctx) const override
    {
        auto estimate = perf_model_.try_estimate_execution_time(c, ctx);

        return hpx::make_ready_future(std::move(estimate));
    }

private:
    caching_cpu_comiler compiler_;
    host_address_space* address_space_;
    abi_info abi_;

    unified_performance_model perf_model_;
};

class cpu_backend final : public host_backend
{
public:
    cpu_backend(const abi_info& abi_)
    : abi_(&abi_),
      address_space_(std::make_unique<host_address_space>(std::make_unique<host_allocator>()))
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

        vpus.push_back(std::make_unique<cpu_vpu>(*address_space_, *abi_));

        return vpus;
    }

    host_address_space& get_host_address_space() override
    {
        return *address_space_;
    }

private:
    const abi_info* abi_;
    std::unique_ptr<host_address_space> address_space_;
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

extern "C" QUBUS_EXPORT host_backend* init_cpu_backend(const abi_info* abi)
{
    std::call_once(cpu_backend_init_flag,
                   [&] { the_cpu_backend = util::make_unique<cpu_backend>(*abi); });

    return the_cpu_backend.get();
}
}
