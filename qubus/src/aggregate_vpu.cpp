#include <qbb/qubus/aggregate_vpu.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

aggregate_vpu::aggregate_vpu(std::unique_ptr<scheduler> scheduler_)
: scheduler_(std::move(scheduler_))
{
}

void aggregate_vpu::add_member_vpu(std::unique_ptr<vpu> new_member_vpu)
{
    member_vpus_.push_back(std:: move(new_member_vpu));
    scheduler_->add_resource(*member_vpus_.back());
}

hpx::future<void> aggregate_vpu::execute(computelet c, execution_context ctx) const
{
    if (member_vpus_.empty())
        throw 0;

    // For now just forward all tasks immediately.
    scheduler_->schedule(std::move(c), std::move(ctx));

    return hpx::make_ready_future();
}

}
}
