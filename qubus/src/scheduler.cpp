#include <qbb/qubus/scheduler.hpp>

#include <boost/optional.hpp>

#include <utility>

namespace qubus
{

void round_robin_scheduler::schedule(computelet c, execution_context ctx)
{
    std::lock_guard<hpx::lcos::local::mutex> guard(scheduling_mutex_);

    auto resource_count = execution_resources_.size();

    auto execution_started = execution_resources_[next_endpoint_]->execute(std::move(c), std::move(ctx));

    next_endpoint_ = (next_endpoint_ + 1) % resource_count;

    execution_started.wait();
}

void round_robin_scheduler::add_resource(vpu& execution_resource)
{
    std::lock_guard<hpx::lcos::local::mutex> guard(scheduling_mutex_);

    execution_resources_.push_back(&execution_resource);
}
}