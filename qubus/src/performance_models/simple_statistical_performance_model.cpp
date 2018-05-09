#include <hpx/config.hpp>

#include <qubus/performance_models/simple_statistical_performance_model.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/error_of.hpp>
#include <boost/accumulators/statistics/error_of_mean.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <hpx/include/local_lcos.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/unused.hpp>

#include <map>
#include <mutex>
#include <utility>
#include <vector>

namespace qubus
{

namespace
{
std::vector<hpx::naming::gid_type> generate_key_from_ctx(const execution_context& ctx)
{
    std::vector<hpx::naming::gid_type> key;
    key.reserve(ctx.args().size());

    for (const auto& obj : ctx.args())
    {
        key.push_back(obj.get_raw_gid());
    }

    return key;
}

class accumulator
{
public:
    using value_type = std::chrono::microseconds::rep;

    void sample(std::chrono::microseconds execution_time)
    {
        acc_(execution_time.count());
    }

    std::chrono::microseconds mean() const
    {
        value_type value = boost::accumulators::mean(acc_);

        return std::chrono::microseconds(std::move(value));
    }

    std::chrono::microseconds variance() const
    {
        value_type value = boost::accumulators::variance(acc_);

        return std::chrono::microseconds(std::move(value));
    }

    std::chrono::microseconds error_of_mean() const
    {
        value_type value = boost::accumulators::error_of<boost::accumulators::tag::mean>(acc_);

        return std::chrono::microseconds(std::move(value));
    }

private:
    boost::accumulators::accumulator_set<
        value_type, boost::accumulators::stats<
                        boost::accumulators::tag::count, boost::accumulators::tag::mean,
                        boost::accumulators::tag::variance,
                        boost::accumulators::tag::error_of<boost::accumulators::tag::mean>>>
        acc_;
};
}

class simple_statistical_performance_model_impl
{
public:
    void sample_execution_time(const symbol_id& QUBUS_UNUSED(func), const execution_context& ctx,
                               std::chrono::microseconds execution_time)
    {
        std::lock_guard<hpx::lcos::local::mutex> guard(accumulators_mutex_);

        auto key = generate_key_from_ctx(ctx);

        auto search_result = accumulators_.find(key);

        if (search_result == accumulators_.end())
        {
            search_result = accumulators_.emplace(key, accumulator()).first;
        }

        search_result->second.sample(std::move(execution_time));
    }

    boost::optional<performance_estimate>
    try_estimate_execution_time(const symbol_id& QUBUS_UNUSED(func), const execution_context& ctx) const
    {
        std::lock_guard<hpx::lcos::local::mutex> guard(accumulators_mutex_);

        auto key = generate_key_from_ctx(ctx);

        auto search_result = accumulators_.find(key);

        if (search_result != accumulators_.end())
        {
            return performance_estimate{search_result->second.mean(),
                                        search_result->second.error_of_mean()};
        }
        else
        {
            return boost::none;
        }
    }

private:
    mutable hpx::lcos::local::mutex accumulators_mutex_;
    std::map<std::vector<hpx::naming::gid_type>, accumulator> accumulators_;
};

simple_statistical_performance_model::simple_statistical_performance_model()
: impl_(std::make_unique<simple_statistical_performance_model_impl>())
{
}

simple_statistical_performance_model::~simple_statistical_performance_model() = default;

void simple_statistical_performance_model::sample_execution_time(
    const symbol_id& func, const execution_context& ctx, std::chrono::microseconds execution_time)
{
    QUBUS_ASSERT(impl_, "Uninitialized object.");

    impl_->sample_execution_time(func, ctx, std::move(execution_time));
}

boost::optional<performance_estimate>
simple_statistical_performance_model::try_estimate_execution_time(
    const symbol_id& func, const execution_context& ctx) const
{
    QUBUS_ASSERT(impl_, "Uninitialized object.");

    return impl_->try_estimate_execution_time(func, ctx);
}
}
