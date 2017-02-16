#ifndef QUBUS_SCHEDULER_HPP
#define QUBUS_SCHEDULER_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/vpu.hpp>

#include <qbb/qubus/computelet.hpp>
#include <qbb/qubus/execution_context.hpp>

#include <hpx/include/lcos.hpp>

#include <vector>
#include <mutex>

namespace qubus
{

class scheduler
{
public:
    scheduler() = default;
    virtual ~scheduler() = default;
    
    scheduler(const scheduler&) = delete;
    scheduler& operator=(const scheduler&) = delete;
    
    virtual void schedule(computelet c, execution_context ctx) = 0;
    virtual void add_resource(vpu& execution_resource) = 0;
};

class round_robin_scheduler final : public scheduler
{
public:
    virtual ~round_robin_scheduler() = default;

    void schedule(computelet c, execution_context ctx) override;
    void add_resource(vpu& execution_resource) override;
private:
    std::vector<vpu*> execution_resources_;
    std::size_t next_endpoint_ = 0;
    mutable hpx::lcos::local::mutex scheduling_mutex_;
};

}

#endif