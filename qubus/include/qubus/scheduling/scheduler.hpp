#ifndef QUBUS_SCHEDULING_SCHEDULER_HPP
#define QUBUS_SCHEDULING_SCHEDULER_HPP

#include <hpx/config.hpp>

#include <qubus/vpu.hpp>

#include <qubus/computelet.hpp>
#include <qubus/execution_context.hpp>

#include <hpx/include/lcos.hpp>

namespace qubus
{

class scheduler
{
public:
    scheduler() = default;
    virtual ~scheduler() = default;
    
    scheduler(const scheduler&) = delete;
    scheduler& operator=(const scheduler&) = delete;
    
    virtual hpx::future<void> schedule(computelet c, execution_context ctx) = 0;
    virtual void add_resource(vpu& execution_resource) = 0;
};

}

#endif