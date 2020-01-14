#ifndef QUBUS_PERFORMANCE_MODELS_PERFORMANCE_MODEL_HPP
#define QUBUS_PERFORMANCE_MODELS_PERFORMANCE_MODEL_HPP

#include <qubus/IR/symbol_id.hpp>
#include <qubus/execution_context.hpp>
#include <qubus/performance_models/performance_estimate.hpp>

#include <boost/optional.hpp>

#include <qubus/util/unused.hpp>

namespace qubus
{

class performance_model
{
public:
    performance_model() = default;
    virtual ~performance_model() = default;

    performance_model(const performance_model&) = delete;
    performance_model& operator=(const performance_model&) = delete;

    performance_model(performance_model&&) = delete;
    performance_model& operator=(performance_model&&) = delete;

    virtual void sample_execution_time(const symbol_id& func, const execution_context& ctx,
                                       std::chrono::microseconds execution_time) = 0;

    virtual boost::optional<performance_estimate>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const = 0;
};
} // namespace qubus

#endif
