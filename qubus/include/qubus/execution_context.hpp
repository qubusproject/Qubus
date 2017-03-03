#ifndef QUBUS_EXECUTION_CONTEXT_HPP
#define QUBUS_EXECUTION_CONTEXT_HPP

#include <qubus/distributed_future.hpp>
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

    execution_context(std::vector<object> args_, std::vector<object> results_,
                      hpx::future<void> is_ready_)
    : arguments_(std::move(args_)),
      results_(std::move(results_)),
      is_ready_(make_distributed_future(hpx::shared_future<void>(std::move(is_ready_))))
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

    hpx::future<void> when_ready() const
    {
        QUBUS_ASSERT(is_ready_.valid(), "Invalid future.");

        return make_future(is_ready_);
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& arguments_;
        ar& results_;
        ar& is_ready_;
    }

private:
    std::vector<object> arguments_;
    std::vector<object> results_;
    distributed_future<void> is_ready_;
};
}

#endif