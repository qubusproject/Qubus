#ifndef QUBUS_EXECUTION_CONTEXT_HPP
#define QUBUS_EXECUTION_CONTEXT_HPP

#include <qubus/object.hpp>

#include <hpx/include/lcos.hpp>

#include <qubus/util/unused.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace qubus
{

class execution_context
{
public:
    execution_context() = default;

    execution_context(std::vector<object> args_, std::vector<object> results_)
    : arguments_(std::move(args_)),
      results_(std::move(results_))
    {
    }

    std::vector<object>& args()
    {
        return arguments_;
    }

    const std::vector<object>& args() const
    {
        return arguments_;
    }

    std::vector<object>& results()
    {
        return results_;
    }

    const std::vector<object>& results() const
    {
        return results_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& arguments_;
        ar& results_;
    }

private:
    std::vector<object> arguments_;
    std::vector<object> results_;
};
}

#endif