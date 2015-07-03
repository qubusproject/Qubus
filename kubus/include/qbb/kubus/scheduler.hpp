#ifndef QBB_KUBUS_SCHEDULER_HPP
#define QBB_KUBUS_SCHEDULER_HPP

#include <hpx/config.hpp>

#include <qbb/kubus/executor.hpp>
#include <qbb/kubus/plan.hpp>
#include <qbb/kubus/execution_context.hpp>

#include <hpx/include/lcos.hpp>

#include <qbb/util/handle.hpp>

#include <unordered_map>

namespace qbb
{
namespace qubus
{

class scheduler
{
public:
    scheduler() = default;
    virtual ~scheduler() = default;
    
    scheduler(const scheduler&) = delete;
    scheduler& operator=(const scheduler&) = delete;
    
    virtual void schedule(const plan& p, execution_context ctx) = 0;
    virtual hpx::shared_future<void> when_ready(const object& obj) = 0;
};

class base_scheduler : public scheduler
{
public:
    virtual ~base_scheduler() = default;
};

class greedy_scheduler final : public base_scheduler
{
public:
    explicit greedy_scheduler(executor& exec_);
    virtual ~greedy_scheduler();
    
    void schedule(const plan& p, execution_context ctx) override;
    hpx::shared_future<void> when_ready(const object& obj) override;
private:
    void purge_object_metadata(const object& obj);
    
    executor* exec_;
    std::unordered_map<util::handle, hpx::shared_future<void>> latest_write_requests_;
    bool is_shutting_down_ = false;
};

}
}

#endif