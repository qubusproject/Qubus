#ifndef QUBUS_KERNEL_PERFORMANCE_MODEL_HPP
#define QUBUS_KERNEL_PERFORMANCE_MODEL_HPP

#include <qubus/IR/symbol_id.hpp>
#include <qubus/execution_context.hpp>
#include <qubus/performance_models/performance_estimate.hpp>

#include <boost/optional.hpp>

#include <qubus/util/unused.hpp>

namespace qubus
{

class kernel_performance_model
{
public:
    kernel_performance_model() = default;
    virtual ~kernel_performance_model() = default;

    kernel_performance_model(const kernel_performance_model&) = delete;
    kernel_performance_model& operator=(const kernel_performance_model&) = delete;

    kernel_performance_model(kernel_performance_model&&) = delete;
    kernel_performance_model& operator=(kernel_performance_model&&) = delete;

    virtual void sample_execution_time(const execution_context& ctx,
                                       std::chrono::microseconds execution_time) = 0;

    virtual boost::optional<performance_estimate>
    try_estimate_execution_time(const execution_context& ctx) const = 0;
};
} // namespace qubus

#endif
