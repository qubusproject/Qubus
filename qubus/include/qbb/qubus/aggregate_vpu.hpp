#ifndef QBB_QUBUS_AGGREGATE_VPU_HPP
#define QBB_QUBUS_AGGREGATE_VPU_HPP

#include <qbb/qubus/vpu.hpp>

#include <hpx/include/components.hpp>

namespace qbb
{
namespace qubus
{

class aggregate_vpu : public vpu_interface, public hpx::components::component_base<aggregate_vpu>
{
public:
    virtual ~aggregate_vpu() = default;

    void add_member_vpu(vpu new_member_vpu);

    void execute(computelet c, execution_context ctx) const override;
private:
    std::vector<vpu> member_vpus_;
};

}
}

#endif
