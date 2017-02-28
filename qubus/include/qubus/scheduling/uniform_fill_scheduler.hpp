#ifndef QUBUS_SCHEDULING_UNIFORM_FILL_SCHEDULER_HPP
#define QUBUS_SCHEDULING_UNIFORM_FILL_SCHEDULER_HPP

#include <qubus/performance_models/performance_model.hpp>
#include <qubus/scheduling/scheduler.hpp>

#include <hpx/include/lcos.hpp>

#include <chrono>
#include <memory>
#include <queue>
#include <vector>

namespace qubus
{

class uniform_fill_scheduler final : public scheduler
{
public:
    ~uniform_fill_scheduler() override = default;

    hpx::future<void> schedule(computelet c, execution_context ctx) override;
    void add_resource(vpu& execution_resource) override;

private:
    struct workload
    {
        explicit workload(vpu& execution_resource)
        : execution_resource(&execution_resource), previous_workload(0)
        {
        }

        vpu* execution_resource;
        std::chrono::microseconds previous_workload;
    };

    struct workload_comperator
    {
    public:
        inline bool operator()(const workload& lhs, const workload& rhs) const
        {
            return lhs.previous_workload > rhs.previous_workload;
        }
    };

    using priority_queue_type = std::priority_queue<workload, std::vector<workload>, workload_comperator>;

    priority_queue_type workloads_;

    std::vector<hpx::future<void>> pending_tasks_;

    mutable hpx::lcos::local::mutex scheduling_mutex_;
};
}

#endif
