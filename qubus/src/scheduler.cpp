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

void greedy_scheduler::schedule(const plan& executed_plan, execution_context ctx)
{
    std::vector<hpx::shared_future<void>> arg_futures;
    std::vector<std::shared_ptr<object>> modified_objects;

    for (const auto& arg : ctx.args() | boost::adaptors::indexed())
    {
        auto arg_future = arg.value()->get_last_modification();

        if (executed_plan.intents().at(arg.index()) == intent::inout)
        {
            modified_objects.push_back(arg.value());
        }

        arg_futures.push_back(arg_future);
    }

    hpx::future<void> deps_ready = hpx::when_all(arg_futures);

    hpx::shared_future<void> task_done =
        deps_ready.then(scheduled_task(executed_plan, std::move(ctx), exec_));

    for (const auto& obj : modified_objects)
    {
        obj->record_modification(task_done);
    }
}

hpx::shared_future<void> greedy_scheduler::when_ready(const object& obj)
{
    return obj.get_last_modification();
}

}
}