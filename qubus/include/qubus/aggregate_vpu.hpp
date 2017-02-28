#ifndef QUBUS_AGGREGATE_VPU_HPP
#define QUBUS_AGGREGATE_VPU_HPP

#include <qubus/scheduling/scheduler.hpp>
#include <qubus/vpu.hpp>

#include <memory>

namespace qubus
{

class aggregate_vpu : public vpu
{
public:
    explicit aggregate_vpu(std::unique_ptr<scheduler> scheduler_);
    virtual ~aggregate_vpu() = default;

    aggregate_vpu(const aggregate_vpu&) = delete;
    aggregate_vpu& operator=(const aggregate_vpu&) = delete;

    aggregate_vpu(aggregate_vpu&&) = default;
    aggregate_vpu& operator=(aggregate_vpu&&) = default;

    void add_member_vpu(std::unique_ptr<vpu> new_member_vpu);

    hpx::future<void> execute(computelet c, execution_context ctx) const override;
    hpx::future<boost::optional<performance_estimate>>
    try_estimate_execution_time(const computelet& c, const execution_context& ctx) const override;
private:
    std::vector<std::unique_ptr<vpu>> member_vpus_;
    std::unique_ptr<scheduler> scheduler_;
};

}

#endif
