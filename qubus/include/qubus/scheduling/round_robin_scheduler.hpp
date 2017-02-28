#ifndef QUBUS_SCHEDULING_ROUND_ROBIN_SCHEDULER_HPP
#define QUBUS_SCHEDULING_ROUND_ROBIN_SCHEDULER_HPP

#include <qubus/scheduling/scheduler.hpp>

#include <hpx/include/lcos.hpp>

#include <vector>

namespace qubus
{

class round_robin_scheduler final : public scheduler
{
public:
    virtual ~round_robin_scheduler() = default;

    hpx::future<void> schedule(computelet c, execution_context ctx) override;

    void add_resource(vpu &execution_resource) override;

private:
    std::vector<vpu *> execution_resources_;
    std::size_t next_endpoint_ = 0;
    mutable hpx::lcos::local::mutex scheduling_mutex_;
};

}

#endif
