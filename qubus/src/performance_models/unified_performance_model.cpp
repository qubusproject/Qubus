#include <qbb/qubus/performance_models/unified_performance_model.hpp>

#include <qbb/qubus/performance_models/regression_performance_model.hpp>
#include <qbb/qubus/performance_models/simple_statistical_performance_model.hpp>

#include <hpx/include/local_lcos.hpp>

#include <qbb/util/assert.hpp>

#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <utility>

inline namespace qbb
{
namespace qubus
{

namespace
{
std::unique_ptr<performance_model> create_performance_model(const computelet& c)
{
    auto code = c.code().get();

    const auto& params = code.params();

    auto has_scalar_params = std::any_of(params.begin(), params.end(), [](const auto& param) {
        const auto& datatype = param.var_type();

        return datatype == types::integer{} || datatype == types::float_{} ||
               datatype == types::double_{};
    });

    if (has_scalar_params)
    {
        return std::make_unique<regression_performance_model>();
    }
    else
    {
        return std::make_unique<simple_statistical_performance_model>();
    }
}
}

class unified_performance_model_impl
{
public:
    void sample_execution_time(const computelet& c, const execution_context& ctx,
                               std::chrono::microseconds execution_time)
    {
        std::lock_guard<hpx::lcos::local::mutex> guard(models_mutex_);

        auto kernel_id = c.id();

        auto search_result = kernel_models_.find(kernel_id);

        if (search_result == kernel_models_.end())
        {
            search_result = kernel_models_.emplace(kernel_id, create_performance_model(c)).first;
        }

        search_result->second->sample_execution_time(c, ctx, std::move(execution_time));
    }

    boost::optional<performance_estimate>
    try_estimate_execution_time(const computelet& c, const execution_context& ctx) const
    {
        std::lock_guard<hpx::lcos::local::mutex> guard(models_mutex_);

        auto kernel_id = c.id();

        auto search_result = kernel_models_.find(kernel_id);

        if (search_result != kernel_models_.end())
        {
            return search_result->second->try_estimate_execution_time(c, ctx);
        }
        else
        {
            return boost::none;
        }
    }

private:
    mutable hpx::lcos::local::mutex models_mutex_;
    std::unordered_map<hpx::naming::gid_type, std::unique_ptr<performance_model>> kernel_models_;
};

unified_performance_model::unified_performance_model()
: impl_(std::make_unique<unified_performance_model_impl>())
{
}

unified_performance_model::~unified_performance_model() = default;

void unified_performance_model::sample_execution_time(const computelet& c,
                                                      const execution_context& ctx,
                                                      std::chrono::microseconds execution_time)
{
    QBB_ASSERT(impl_, "Uninitialized object.");

    impl_->sample_execution_time(c, ctx, std::move(execution_time));
}

boost::optional<performance_estimate>
unified_performance_model::try_estimate_execution_time(const computelet& c,
                                                       const execution_context& ctx) const
{
    QBB_ASSERT(impl_, "Uninitialized object.");

    return impl_->try_estimate_execution_time(c, ctx);
}
}
}