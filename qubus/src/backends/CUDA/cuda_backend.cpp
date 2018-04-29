#include <qubus/backend.hpp>

#include <qubus/backend.hpp>

#include <qubus/backends/cuda/cuda_allocator.hpp>
#include <qubus/backends/cuda/cuda_compiler.hpp>

#include <qubus/abi_info.hpp>
#include <qubus/local_address_space.hpp>
#include <qubus/module_library.hpp>
#include <qubus/performance_models/unified_performance_model.hpp>

#include <qubus/qubus_export.h>

#include <qubus/cuda/core.hpp>
#include <qubus/cuda/support.hpp>

#include <hpx/async.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/thread_executors.hpp>
#include <hpx/include/threads.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>

#include <qubus/IR/qir.hpp>

namespace qubus
{

namespace
{

class caching_cuda_comiler
{
public:
    explicit caching_cuda_comiler(module_library mod_library_)
    : mod_library_(std::move(mod_library_))
    {
    }

    const cuda_plan& compile(const symbol_id& func) const
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

private:
    module_library mod_library_;
    mutable cuda_compiler underlying_compiler_; // FIXME: Make the compiler non-mutable.
    mutable std::unordered_map<symbol_id, std::unique_ptr<cuda_plan>> compilation_cache_;
    mutable hpx::lcos::local::mutex cache_mutex_;
};

class cuda_vpu final : public vpu
{
public:
    explicit cuda_vpu(cuda::device dev_, local_address_space& local_addr_space_,
                      module_library mod_library_)
    : compiler_(mod_library_),
      dev_(std::move(dev_)),
      cuda_ctx_(this->dev_),
      address_space_(std::make_unique<cuda_allocator>(this->cuda_ctx_)),
      perf_model_(mod_library_)
    {
        auto page_fault_handler =
            [this, &local_addr_space_](
                const object& obj,
                address_space::page_fault_context ctx) -> hpx::future<address_space::handle> {
            auto host_handle = local_addr_space_.resolve_object(obj);

            hpx::threads::executors::pool_executor service_executor("/qubus/service");

            return host_handle.then(
                service_executor,
                [this, &obj, ctx](hpx::future<host_address_space::handle> host_handle) mutable {
                    auto handle = host_handle.get();

                    auto size = handle.data().size();

                    auto page = ctx.allocate_page(size, 1);

                    auto data = page.data().ptr();

                    cuda::device_ptr ptr;

                    static_assert(
                        sizeof(ptr) == sizeof(void*),
                        "The size of a CUDA device pointer has match the size of a host pointer.");

                    std::memcpy(&ptr, &data, sizeof(void*));

                    cuda::async_memcpy(ptr, handle.data().ptr(), size, stream_);

                    hpx::threads::executors::pool_executor service_executor("/qubus/service");

                    return cuda::when_finished(stream_).then(
                        service_executor,
                        [page = std::move(page)](hpx::future<void> when_finished) mutable {
                            when_finished.get();

                            return std::move(page);
                        });
                });
        };

        address_space_.on_page_fault(std::move(page_fault_handler));
    }

    hpx::future<void> execute(const symbol_id& func, execution_context ctx) override
    {
        cuda::context_guard guard(cuda_ctx_);

        const auto& compilation = compiler_.compile(func);

        hpx::threads::executors::pool_executor service_executor("/qubus/service");

        hpx::future<void> task_done =
            hpx::async(service_executor, [this, &compilation, func, ctx]() mutable {
                std::vector<void*> task_args;

                std::vector<address_space::handle> pages;

                for (const auto& arg : ctx.args())
                {
                    auto page = address_space_.resolve_object(arg).get();

                    task_args.push_back(page.data().ptr());

                    pages.push_back(std::move(page));
                }

                for (const auto& result : ctx.results())
                {
                    auto page = address_space_.resolve_object(result).get();

                    task_args.push_back(page.data().ptr());

                    pages.push_back(std::move(page));
                }

                cuda::context_guard guard(cuda_ctx_);

                cuda::event start_exec(stream_);
                compilation.execute(func, task_args, stream_);
                cuda::event end_exec(stream_);

                guard.deactivate();

                auto execution_finished = when_finished(stream_);

                execution_finished.get();

                auto task_duration = std::chrono::duration_cast<std::chrono::microseconds>(
                    cuda::compute_elapsed_time(start_exec, end_exec));

                perf_model_.sample_execution_time(func, ctx, std::move(task_duration));
            });

        return task_done;
    }

    hpx::future<boost::optional<performance_estimate>>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const override
    {
        auto estimate = perf_model_.try_estimate_execution_time(func, ctx);

        return hpx::make_ready_future(std::move(estimate));
    }

private:
    caching_cuda_comiler compiler_;
    cuda::device dev_;
    cuda::context cuda_ctx_;
    cuda::stream stream_;

    address_space address_space_;
    abi_info abi_;

    unified_performance_model perf_model_;
};

class cuda_backend final : public backend
{
public:
    cuda_backend(const abi_info& abi_, local_address_space& local_addr_space_,
                 module_library mod_library_)
    : abi_(&abi_), local_addr_space_(&local_addr_space_), mod_library_(std::move(mod_library_))
    {
    }

    ~cuda_backend() override = default;

    std::string id() const override
    {
        return "qubus.cuda";
    }

    std::vector<std::unique_ptr<vpu>> create_vpus() const override
    {
        try
        {
            cuda::init();

            std::vector<std::unique_ptr<vpu>> vpus;

            for (auto&& device : cuda::get_devices())
            {
                vpus.push_back(std::make_unique<cuda_vpu>(std::move(device), *local_addr_space_,
                                                          mod_library_));
            }

            return vpus;
        }
        catch (const cuda::cuda_error&)
        {
            // TODO: Log error

            return {};
        }
    }

private:
    const abi_info* abi_;
    local_address_space* local_addr_space_;
    module_library mod_library_;
};
} // namespace

extern "C" QUBUS_EXPORT unsigned int cuda_backend_get_backend_type()
{
    return static_cast<unsigned int>(backend_type::vpu);
}

extern "C" QUBUS_EXPORT unsigned long int cuda_backend_get_api_version()
{
    return 0;
}

namespace
{
std::unique_ptr<cuda_backend> the_cuda_backend;
std::once_flag cuda_backend_init_flag;
} // namespace

extern "C" QUBUS_EXPORT backend* init_cuda_backend(const abi_info* abi,
                                                   local_address_space* local_addr_space,
                                                   module_library mod_library)
{
    QUBUS_ASSERT(abi, "Expected argument to be non-null.");
    QUBUS_ASSERT(local_addr_space, "Expected argument to be non-null.");

    std::call_once(cuda_backend_init_flag, [&] {
        the_cuda_backend =
            std::make_unique<cuda_backend>(*abi, *local_addr_space, std::move(mod_library));
    });

    return the_cuda_backend.get();
}
} // namespace qubus