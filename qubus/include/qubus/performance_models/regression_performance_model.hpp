#ifndef QUBUS_PERFORMANCE_MODELS_REGRESSION_PERFORMANCE_MODEL_HPP
#define QUBUS_PERFORMANCE_MODELS_REGRESSION_PERFORMANCE_MODEL_HPP

#include <qubus/performance_models/performance_model.hpp>

#include <memory>

namespace qubus
{

class address_space;

class regression_performance_model_impl;

class regression_performance_model final : public performance_model
{
public:
    explicit regression_performance_model(address_space& host_addr_space_);
    virtual ~regression_performance_model();

    void sample_execution_time(const symbol_id& func, const execution_context& ctx,
                               std::chrono::microseconds execution_time) override;

    boost::optional<performance_estimate>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const override;
private:
    std::unique_ptr<regression_performance_model_impl> impl_;
};

}

#endif
