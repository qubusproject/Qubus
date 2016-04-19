#include <qbb/qubus/aggregate_vpu.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

void aggregate_vpu::add_member_vpu(vpu new_member_vpu)
{
    member_vpus_.push_back(std:: move(new_member_vpu));
}

void aggregate_vpu::execute(computelet c, execution_context ctx) const
{
    if (member_vpus_.empty())
        throw 0;

    member_vpus_.front().execute(std::move(c), std::move(ctx));
}

}
}

using server_type = hpx::components::component<qbb::qubus::aggregate_vpu>;
HPX_REGISTER_COMPONENT(server_type, qbb_qubus_aggregate_vpu);