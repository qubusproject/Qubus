#ifndef QBB_QUBUS_EXECUTOR_HPP
#define QBB_QUBUS_EXECUTOR_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/plan.hpp>
#include <qbb/qubus/execution_context.hpp>

#include <hpx/lcos/future.hpp>

#include <string>

namespace qbb
{
namespace qubus
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