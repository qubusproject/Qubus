#include <qubus/scheduling/uniform_fill_scheduler.hpp>

#include <qubus/util/assert.hpp>

#include <utility>

namespace qubus
{

hpx::future<void> uniform_fill_scheduler::schedule(computelet c, execution_context ctx)
{
    auto is_ready = ctx.when_ready();

    auto finished = is_ready.then([this, c, ctx](hpx::future<void> is_ready) {
        is_ready.get();

        std::unique_lock<hpx::lcos::local::mutex> guard(scheduling_mutex_);

        auto least_workload = workloads_.top();
        workloads_.pop();

        auto estimate =
            least_workload.execution_resource->try_estimate_execution_time(c, ctx).get();

        if (estimate)
        {
            auto runtime = estimate->runtime;

            auto previous_workload_ticks = least_workload.previous_workload.count();
            auto runtime_ticks = runtime.count();

            decltype(previous_workload_ticks) new_workload_ticks;

            if (__builtin_add_overflow(previous_workload_ticks, runtime_ticks, &new_workload_ticks))
            {
                priority_queue_type rescaled_workloads;

                while (!workloads_.empty())
                {
                    auto workload = workloads_.top();
                    workloads_.pop();

                    workload.previous_workload -= least_workload.previous_workload;

                    rescaled_workloads.push(std::move(workload));
                }

                workloads_ = std::move(rescaled_workloads);

                least_workload.previous_workload = std::chrono::microseconds(0);
            }
            else
            {
                least_workload.previous_workload =
                    std::chrono::microseconds(std::move(new_workload_ticks));
            }
        }

        workloads_.push(std::move(least_workload));

        guard.unlock();

        return least_workload.execution_resource->execute(c, ctx);
    });

    return finished;
}

void uniform_fill_scheduler::add_resource(vpu& execution_resource)
{
    std::lock_guard<hpx::lcos::local::mutex> guard(scheduling_mutex_);

    workloads_.push(workload(execution_resource));
}
}
