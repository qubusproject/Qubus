#include <qubus/backend.hpp>

#include <qubus/local_runtime.hpp>

#include <qubus/backend.hpp>

#include <qubus/backends/cuda/cuda_allocator.hpp>
#include <qubus/backends/cuda/cuda_compiler.hpp>

#include <qubus/abi_info.hpp>
#include <qubus/local_address_space.hpp>
#include <qubus/performance_models/unified_performance_model.hpp>

#include <qubus/qubus_export.h>

#include <qubus/cuda/core.hpp>
#include <qubus/cuda/support.hpp>

#include <hpx/async.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/threads.hpp>

#include <qubus/hpx_utils.hpp>

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
    const cuda_plan& compile(const computelet& c) const
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
    mutable cuda_compiler underlying_compiler_; // FIXME: Make the compiler non-mutable.
    mutable std::unordered_map<hpx::naming::gid_type, std::unique_ptr<cuda_plan>>
        compilation_cache_;
    mutable hpx::lcos::local::mutex cache_mutex_;
};

class cuda_vpu final : public vpu
{
public:
    explicit cuda_vpu(cuda::device dev_)
    : dev_(std::move(dev_)),
      cuda_ctx_(this->dev_),
      address_space_(std::make_unique<cuda_allocator>(this->cuda_ctx_))
    {
    }

    hpx::future<void> execute(computelet c, execution_context ctx) override
    {
        cuda::context_guard guard(cuda_ctx_);

        const auto& compilation = compiler_.compile(c);

        auto deps_ready = ctx.when_ready();

        hpx::future<void> task_done = deps_ready.then(
            get_local_runtime().get_service_executor(), [this, &compilation, c, ctx](auto) mutable {
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
                compilation.execute(task_args, stream_);
                cuda::event end_exec(stream_);

                guard.deactivate();

                auto execution_finished = when_finished(stream_);

                execution_finished.get();

                auto task_duration = std::chrono::duration_cast<std::chrono::microseconds>(
                    cuda::compute_elapsed_time(start_exec, end_exec));

                perf_model_.sample_execution_time(c, ctx, std::move(task_duration));
            });

        return task_done;
    }

    hpx::future<boost::optional<performance_estimate>>
    try_estimate_execution_time(const computelet& c, const execution_context& ctx) const override
    {
        auto estimate = perf_model_.try_estimate_execution_time(c, ctx);

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
    cuda_backend(const abi_info& abi_) : abi_(&abi_)
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

            for (auto &&device : cuda::get_devices())
            {
                vpus.push_back(std::make_unique<cuda_vpu>(std::move(device)));
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
};
}

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
}

extern "C" QUBUS_EXPORT backend* init_cuda_backend(const abi_info* abi)
{
    std::call_once(cuda_backend_init_flag,
                   [&] { the_cuda_backend = std::make_unique<cuda_backend>(*abi); });

    return the_cuda_backend.get();
}
}