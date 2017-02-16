#ifndef QUBUS_PERFORMANCE_MODELS_SIMPLE_STATISTICAL_PERFORMANCE_MODEL_HPP
#define QUBUS_PERFORMANCE_MODELS_SIMPLE_STATISTICAL_PERFORMANCE_MODEL_HPP

#include <qbb/qubus/performance_models/performance_model.hpp>

#include <memory>

namespace qubus
{
class simple_statistical_performance_model_impl;

class simple_statistical_performance_model final : public performance_model
{
public:
    simple_statistical_performance_model();
    virtual ~simple_statistical_performance_model();

    void sample_execution_time(const computelet& c, const execution_context& ctx,
                               std::chrono::microseconds execution_time) override;

    boost::optional<performance_estimate>
    try_estimate_execution_time(const computelet& c, const execution_context& ctx) const override;
private:
    std::unique_ptr<simple_statistical_performance_model_impl> impl_;
};
}

#endif
