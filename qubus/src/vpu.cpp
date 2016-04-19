#include <qbb/qubus/vpu.hpp>

#include <utility>

HPX_DEFINE_GET_COMPONENT_TYPE(qbb::qubus::vpu_interface);

using execute_action = qbb::qubus::vpu_interface::execute_action;
HPX_REGISTER_ACTION_DECLARATION(execute_action);
HPX_REGISTER_ACTION(execute_action);

namespace qbb
{
namespace qubus
{

void vpu_interface::execute_nonvirt(computelet c, execution_context ctx) const
{
    execute(std::move(c), std::move(ctx));
}

vpu::vpu(hpx::future<hpx::id_type>&& id)
: base_type(std::move(id))
{
}

void vpu::execute(computelet c, execution_context ctx) const
{
    hpx::apply<vpu_interface::execute_action>(this->get_id(), std::move(c), std::move(ctx));
}

}
}
