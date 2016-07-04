#ifndef QBB_QUBUS_VPU_HPP
#define QBB_QUBUS_VPU_HPP

#include <qbb/qubus/computelet.hpp>
#include <qbb/qubus/execution_context.hpp>

#include <hpx/include/components.hpp>

#include <memory>

namespace qbb
{
namespace qubus
{

class vpu
{
public:
    vpu() = default;

    virtual ~vpu() = default;

    virtual hpx::future<void> execute(computelet c, execution_context ctx) const = 0;

protected:
    vpu(const vpu &) = default;

    vpu &operator=(const vpu &) = default;

    vpu(vpu &&) = default;

    vpu &operator=(vpu &&) = default;
};

class remote_vpu_server : public hpx::components::component_base<remote_vpu_server>
{
public:
    remote_vpu_server() = default;

    explicit remote_vpu_server(std::unique_ptr<vpu> underlying_vpu_);

    void execute(computelet c, execution_context ctx) const;

    HPX_DEFINE_COMPONENT_ACTION(remote_vpu_server, execute, execute_action);

private:
    std::unique_ptr<vpu> underlying_vpu_;
};

class remote_vpu : public vpu, public hpx::components::client_base<remote_vpu, remote_vpu_server>
{
public:
    using base_type = hpx::components::client_base<remote_vpu, remote_vpu_server>;

    remote_vpu() = default;

    remote_vpu(hpx::future<hpx::id_type> &&id);

    hpx::future<void> execute(computelet c, execution_context ctx) const;
};

class remote_vpu_reference_server : public hpx::components::component_base<remote_vpu_reference_server>
{
public:
    remote_vpu_reference_server() = default;

    explicit remote_vpu_reference_server(vpu *underlying_vpu_);

    void execute(computelet c, execution_context ctx) const;

    HPX_DEFINE_COMPONENT_ACTION(remote_vpu_reference_server, execute, execute_action);

private:
    vpu *underlying_vpu_;
};

class remote_vpu_reference
        : public vpu, public hpx::components::client_base<remote_vpu_reference, remote_vpu_reference_server>
{
public:
    using base_type = hpx::components::client_base<remote_vpu_reference, remote_vpu_reference_server>;

    remote_vpu_reference() = default;

    remote_vpu_reference(hpx::future<hpx::id_type> &&id);

    hpx::future<void> execute(computelet c, execution_context ctx) const;
};

}
}

#endif
