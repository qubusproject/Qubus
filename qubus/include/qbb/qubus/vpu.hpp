#ifndef QBB_QUBUS_VPU_HPP
#define QBB_QUBUS_VPU_HPP

#include <qbb/qubus/computelet.hpp>
#include <qbb/qubus/execution_context.hpp>

#include <hpx/include/components.hpp>

namespace qbb
{
namespace qubus
{

class vpu_interface : public hpx::components::abstract_component_base<vpu_interface>
{
public:
    vpu_interface() = default;
    virtual ~vpu_interface() = default;

    vpu_interface(const vpu_interface&) = delete;
    vpu_interface& operator=(const vpu_interface&) = delete;

    vpu_interface(vpu_interface&&) = delete;
    vpu_interface& operator=(vpu_interface&&) = delete;

    virtual void execute(computelet c, execution_context ctx) const = 0;

    void execute_nonvirt(computelet c, execution_context ctx) const;
    HPX_DEFINE_COMPONENT_ACTION(vpu_interface, execute_nonvirt, execute_action);
};

class vpu : public hpx::components::client_base<vpu, vpu_interface>
{
public:
    using base_type = hpx::components::client_base<vpu, vpu_interface>;

    vpu() = default;
    vpu(hpx::future<hpx::id_type>&& id);

    void execute(computelet c, execution_context ctx) const;
};

}
}

#endif
