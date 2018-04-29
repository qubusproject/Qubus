#include <qubus/aggregate_vpu.hpp>

#include <boost/range/adaptor/indirected.hpp>

#include <utility>

namespace qubus
{

aggregate_vpu::aggregate_vpu(std::unique_ptr<scheduler> scheduler_)
: scheduler_(std::move(scheduler_))
{
}

void aggregate_vpu::add_member_vpu(std::unique_ptr<vpu> new_member_vpu)
{
    member_vpus_.push_back(std::move(new_member_vpu));
    scheduler_->add_resource(*member_vpus_.back());
}

hpx::future<void> aggregate_vpu::execute(const symbol_id& func, execution_context ctx)
{
    if (member_vpus_.empty())
        throw 0;

    // For now just forward all tasks immediately.
    return scheduler_->schedule(func, std::move(ctx));
}

hpx::future<boost::optional<performance_estimate>>
aggregate_vpu::try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const
{
    // We will just forward the task and can simply base our estimate on the estimates of
    // the member VPUs.
    //
    // Since the estimate should be as close to the actual value **on average**, we simply use
    // the average of all collected estimates. This will only be valid if the work is distributed
    // uniformly over all member VPUs which should be justified if the performance of all member VPUs
    // is almost equal.
    //
    // Note: To improve the estimate, we could memorize the frequency of selecting each VPU and
    //       use these values to compute a weighted average instead.

    std::vector<hpx::future<boost::optional<performance_estimate>>> collected_estimates;

    for (const auto& vpu : member_vpus_ | boost::adaptors::indirected)
    {
        auto estimate = vpu.try_estimate_execution_time(func, ctx);

        collected_estimates.push_back(std::move(estimate));
    }

    auto estimate =
        hpx::when_all(collected_estimates)
            .then([](hpx::future<std::vector<hpx::future<boost::optional<performance_estimate>>>>
                         collected_estimates) {
                auto average_estimate = std::chrono::microseconds::zero();
                auto min_accuracy = std::chrono::microseconds::zero();

                for (auto& estimate : collected_estimates.get())
                {
                    auto value = estimate.get();

                    if (!value)
                        return boost::optional<performance_estimate>();

                    average_estimate += value->runtime;
                    min_accuracy = std::max(min_accuracy, value->accuracy);
                }

                average_estimate /= collected_estimates.get().size();

                return boost::optional<performance_estimate>(
                    performance_estimate{std::move(average_estimate), std::move(min_accuracy)});
            });

    return estimate;
}
}
