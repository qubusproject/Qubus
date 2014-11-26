#ifndef QBB_KUBUS_EXECUTOR_HPP
#define QBB_KUBUS_EXECUTOR_HPP

#include <hpx/config.hpp>

#include <qbb/kubus/plan.hpp>
#include <qbb/kubus/execution_context.hpp>

#include <hpx/lcos/future.hpp>

#include <string>

namespace qbb
{
namespace kubus
{

class executor
{
public:
    executor() = default;
    executor(const executor&) = delete;
    
    virtual ~executor() = default;
    
    executor& operator=(const executor&) = delete;
    
    //TODO: Substitute string with string_view
    //virtual const std::string& id() const = 0;
    
    virtual hpx::lcos::future<void> execute_plan(const plan& executed_plan, execution_context ctx) = 0;
};
    
}   
}

#endif