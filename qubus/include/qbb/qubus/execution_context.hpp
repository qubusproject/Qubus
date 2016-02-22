#ifndef QBB_QUBUS_EXECUTION_CONTEXT_HPP
#define QBB_QUBUS_EXECUTION_CONTEXT_HPP

#include <qbb/qubus/object.hpp>

#include <qbb/util/unused.hpp>

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
    execution_context() = default;

    explicit execution_context(std::vector<std::shared_ptr<object>> arguments_)
    : arguments_(std::move(arguments_))
    {
    }

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

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & arguments_;
    }
private:
    std::vector<std::shared_ptr<object>> arguments_;
};
    
}
}

#endif