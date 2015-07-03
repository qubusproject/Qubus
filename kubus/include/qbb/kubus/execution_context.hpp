#ifndef QBB_KUBUS_EXECUTION_CONTEXT_HPP
#define QBB_KUBUS_EXECUTION_CONTEXT_HPP

#include <qbb/kubus/object.hpp>

#include <vector>
#include <memory>
#include <utility>

namespace qbb
{
namespace qubus
{

class execution_context
{
public:
    explicit execution_context(std::vector<std::shared_ptr<object>> arguments_)
    : arguments_(std::move(arguments_))
    {
    }
    
    execution_context() = default;
    execution_context(const execution_context&) = delete;
    execution_context(execution_context&&) = default;
    
    execution_context& operator=(const execution_context&) = delete;
    execution_context& operator=(execution_context&&) = default;
    
    const std::vector<std::shared_ptr<object>>& args() const
    {
        return arguments_;
    }
    
    void push_back_arg(std::shared_ptr<object> arg)
    {
        arguments_.push_back(arg);
    }
private:
    std::vector<std::shared_ptr<object>> arguments_;
};
    
}
}

#endif