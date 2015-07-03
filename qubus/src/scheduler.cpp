#include <qbb/qubus/scheduler.hpp>

#include <boost/range/adaptor/indexed.hpp>

#include <boost/optional.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

namespace
{
class scheduled_task
{
public:
    scheduled_task(plan executed_plan_, execution_context ctx_, executor* exec_)
    : executed_plan_(std::move(executed_plan_)), ctx_(std::move(ctx_)), exec_(std::move(exec_))
    {
    }

    hpx::lcos::future<void> operator()(const hpx::future<void>&)
    {
        return exec_->execute_plan(executed_plan_, std::move(ctx_));
    }

private:
    plan executed_plan_;
    execution_context ctx_;
    executor* exec_;
};
}

greedy_scheduler::greedy_scheduler(executor& exec_) : exec_(&exec_)
{
}

greedy_scheduler::~greedy_scheduler()
{
    is_shutting_down_ = true;

    for (const auto& pair : latest_write_requests_)
    {
        pair.second.wait();
    }
}

void greedy_scheduler::schedule(const plan& executed_plan, execution_context ctx)
{
    std::vector<hpx::shared_future<void>> arg_futures;

    std::vector<hpx::shared_future<void>*> result_futures;

    for (const auto& arg : ctx.args() | boost::adaptors::indexed())
    {
        auto iter_success =
            latest_write_requests_.emplace(arg.value()->id(), hpx::make_ready_future());

        if (executed_plan.intents().at(arg.index()) == intent::inout)
        {
            result_futures.push_back(&iter_success.first->second);
        }

        if (iter_success.second)
        {
            arg.value()->on_destruction([this](const object& obj)
                                       {
                                           purge_object_metadata(obj);
                                       });
        }

        auto iter = iter_success.first;

        auto& arg_future = iter->second;

        arg_futures.push_back(arg_future);
    }

    hpx::future<void> deps_ready = hpx::when_all(arg_futures);

    hpx::shared_future<void> task_done =
        deps_ready.then(scheduled_task(executed_plan, std::move(ctx), exec_));

    for (auto future : result_futures)
    {
       *future = task_done;
    }
}

hpx::shared_future<void> greedy_scheduler::when_ready(const object& obj)
{
    auto iter_success = latest_write_requests_.emplace(obj.id(), hpx::make_ready_future());

    if (iter_success.second)
    {
        obj.on_destruction([this](const object& obj)
                           {
                               purge_object_metadata(obj);
                           });
    }

    auto iter = iter_success.first;

    return iter->second;
}

void greedy_scheduler::purge_object_metadata(const object& obj)
{
    latest_write_requests_.erase(obj.id());
}
}
}