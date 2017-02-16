#ifndef QUBUS_AGGREGATE_VPU_HPP
#define QUBUS_AGGREGATE_VPU_HPP

#include <qbb/qubus/scheduler.hpp>
#include <qbb/qubus/vpu.hpp>

#include <memory>

namespace qubus
{

class aggregate_vpu : public vpu
{
public:
    explicit aggregate_vpu(std::unique_ptr<scheduler> scheduler_);
    virtual ~aggregate_vpu() = default;

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
