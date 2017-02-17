#ifndef QUBUS_PERFORMANCE_MODELS_UNIFIED_PERFORMANCE_MODEL_HPP
#define QUBUS_PERFORMANCE_MODELS_UNIFIED_PERFORMANCE_MODEL_HPP

#include <qubus/performance_models/performance_model.hpp>

#include <memory>

namespace qubus
{

class unified_performance_model_impl;

class unified_performance_model final : public performance_model
{
public:
    unified_performance_model();
    virtual ~unified_performance_model();

    void sample_execution_time(const computelet& c, const execution_context& ctx,
                               std::chrono::microseconds execution_time) override;

    boost::optional<performance_estimate>
    try_estimate_execution_time(const computelet& c, const execution_context& ctx) const override;

private:
    std::unique_ptr<unified_performance_model_impl> impl_;
};

}

#endif
