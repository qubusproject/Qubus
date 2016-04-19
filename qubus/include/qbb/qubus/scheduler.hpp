#ifndef QBB_QUBUS_SCHEDULER_HPP
#define QBB_QUBUS_SCHEDULER_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/execution_context.hpp>

#include <hpx/include/lcos.hpp>

#include <qbb/util/handle.hpp>

#include <unordered_map>

namespace qbb
{
namespace qubus
{

/*class scheduler
{
public:
    scheduler() = default;
    virtual ~scheduler() = default;
    
    scheduler(const scheduler&) = delete;
    scheduler& operator=(const scheduler&) = delete;
    
    virtual void schedule(const plan& p, execution_context ctx) = 0;
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
    virtual ~greedy_scheduler() = default;
    
    void schedule(const plan& p, execution_context ctx) override;
private:
    executor* exec_;
};*/

}
}

#endif