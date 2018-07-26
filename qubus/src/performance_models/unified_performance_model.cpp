#include <hpx/config.hpp>

#include <qubus/performance_models/unified_performance_model.hpp>

#include <qubus/performance_models/regression_performance_model.hpp>
#include <qubus/performance_models/simple_statistical_performance_model.hpp>

#include <hpx/include/local_lcos.hpp>

#include <qubus/util/assert.hpp>

#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace qubus
{

namespace
{
std::unique_ptr<performance_model> create_performance_model(const symbol_id& func,
                                                            const module_library& mod_library,
                                                            address_space& host_addr_space)
{
    auto mod = mod_library.lookup(func.get_prefix()).get();

    const function& func_def = mod->lookup_function(func.suffix());

    const auto& params = func_def.params();

    auto has_scalar_params = std::any_of(params.begin(), params.end(), [](const auto& param) {
        const auto& datatype = param.var_type();

        return datatype == types::integer{} || datatype == types::float_{} ||
               datatype == types::double_{};
    });

    if (has_scalar_params)
    {
        return std::make_unique<regression_performance_model>(host_addr_space);
    }
    else
    {
        return std::make_unique<simple_statistical_performance_model>();
    }
}
} // namespace

class unified_performance_model_impl
{
public:
    explicit unified_performance_model_impl(module_library mod_library_,
                                            address_space& host_addr_space_)
    : mod_library_(std::move(mod_library_)), host_addr_space_(&host_addr_space_)
    {
    }

    void sample_execution_time(const symbol_id& func, const execution_context& ctx,
                               std::chrono::microseconds execution_time)
    {
        std::lock_guard<hpx::lcos::local::mutex> guard(models_mutex_);

        auto search_result = kernel_models_.find(func);

        if (search_result == kernel_models_.end())
        {
            search_result =
                kernel_models_
                    .emplace(func, create_performance_model(func, mod_library_, *host_addr_space_))
                    .first;
        }

        search_result->second->sample_execution_time(func, ctx, std::move(execution_time));
    }

    boost::optional<performance_estimate>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const
    {
        std::lock_guard<hpx::lcos::local::mutex> guard(models_mutex_);

        auto search_result = kernel_models_.find(func);

        if (search_result != kernel_models_.end())
        {
            return search_result->second->try_estimate_execution_time(func, ctx);
        }
        else
        {
            return boost::none;
        }
    }

private:
    mutable hpx::lcos::local::mutex models_mutex_;
    std::unordered_map<symbol_id, std::unique_ptr<performance_model>> kernel_models_;

    module_library mod_library_;
    address_space* host_addr_space_;
};

unified_performance_model::unified_performance_model(module_library mod_library_,
                                                     address_space& host_addr_space_)
: impl_(std::make_unique<unified_performance_model_impl>(std::move(mod_library_), host_addr_space_))
{
}

unified_performance_model::~unified_performance_model() = default;

void unified_performance_model::sample_execution_time(const symbol_id& func,
                                                      const execution_context& ctx,
                                                      std::chrono::microseconds execution_time)
{
    QUBUS_ASSERT(impl_, "Uninitialized object.");

    impl_->sample_execution_time(func, ctx, std::move(execution_time));
}

boost::optional<performance_estimate>
unified_performance_model::try_estimate_execution_time(const symbol_id& func,
                                                       const execution_context& ctx) const
{
    QUBUS_ASSERT(impl_, "Uninitialized object.");

    return impl_->try_estimate_execution_time(func, ctx);
}
} // namespace qubus