#ifndef QUBUS_PERFORMANCE_MODELS_UNIFIED_PERFORMANCE_MODEL_HPP
#define QUBUS_PERFORMANCE_MODELS_UNIFIED_PERFORMANCE_MODEL_HPP

#include <qubus/module_library.hpp>
#include <qubus/performance_models/performance_model.hpp>

#include <memory>

namespace qubus
{

class unified_performance_model_impl;

class unified_performance_model final : public performance_model
{
public:
    explicit unified_performance_model(module_library mod_library_);
    virtual ~unified_performance_model();

    void sample_execution_time(const symbol_id& func, const execution_context& ctx,
                               std::chrono::microseconds execution_time) override;

    boost::optional<performance_estimate>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const override;

private:
    std::unique_ptr<unified_performance_model_impl> impl_;
};

}

#endif
