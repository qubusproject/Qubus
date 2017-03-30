#ifndef QUBUS_KERNEL_ARGUMENTS_HPP
#define QUBUS_KERNEL_ARGUMENTS_HPP

#include <qubus/object.hpp>

#include <qubus/util/unused.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace qubus
{

class kernel_arguments
{
public:
    kernel_arguments() = default;

    std::vector<object>& args()
    {
        return arguments_;
    }

    const std::vector<object>& args() const
    {
        return arguments_;
    }

    void push_back_arg(object arg)
    {
        arguments_.push_back(arg);
    }

    std::vector<object>& results()
    {
        return results_;
    }

    const std::vector<object>& results() const
    {
        return results_;
    }

    void push_back_result(object result)
    {
        results_.push_back(result);
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
