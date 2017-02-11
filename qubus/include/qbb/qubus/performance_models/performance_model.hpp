#ifndef QUBUS_PERFORMANCE_MODEL_HPP
#define QUBUS_PERFORMANCE_MODEL_HPP

#include <qbb/qubus/computelet.hpp>
#include <qbb/qubus/execution_context.hpp>

#include <boost/optional.hpp>

#include <chrono>

namespace qbb
{
namespace qubus
{

struct performance_estimate
{
    std::chrono::microseconds runtime;
    std::chrono::microseconds accuracy;
};

class performance_model
{
public:
    performance_model() = default;
    virtual ~performance_model() = default;

    performance_model(const performance_model&) = delete;
    performance_model& operator=(const performance_model&) = delete;

    performance_model(performance_model&&) = delete;
    performance_model& operator=(performance_model&&) = delete;

    virtual void sample_execution_time(const computelet& c, const execution_context& ctx,
                                       std::chrono::microseconds execution_time) = 0;

    virtual boost::optional<performance_estimate>
    try_estimate_execution_time(const computelet& c, const execution_context& ctx) const = 0;
};
}
}

#endif
