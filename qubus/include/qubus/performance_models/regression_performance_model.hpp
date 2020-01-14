#ifndef QUBUS_PERFORMANCE_MODELS_REGRESSION_PERFORMANCE_MODEL_HPP
#define QUBUS_PERFORMANCE_MODELS_REGRESSION_PERFORMANCE_MODEL_HPP

#include <qubus/performance_models/kernel_performance_model.hpp>
#include <qubus/IR/function.hpp>
#include <qubus/local_address_space.hpp>

#include <memory>

namespace qubus
{

class regression_performance_model_impl;

class regression_performance_model final : public kernel_performance_model
{
public:
    regression_performance_model(const function& fn_, host_address_space& host_addr_space_);
    virtual ~regression_performance_model();

    void sample_execution_time(const execution_context& ctx,
                               std::chrono::microseconds execution_time) override;

    boost::optional<performance_estimate>
    try_estimate_execution_time(const execution_context& ctx) const override;

private:
    std::unique_ptr<regression_performance_model_impl> impl_;
};

} // namespace qubus

#endif
