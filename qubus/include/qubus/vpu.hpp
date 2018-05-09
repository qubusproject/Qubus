#ifndef QUBUS_VPU_HPP
#define QUBUS_VPU_HPP

#include <qubus/performance_models/performance_model.hpp>

#include <qubus/IR/symbol_id.hpp>
#include <qubus/execution_context.hpp>

#include <hpx/include/components.hpp>

#include <boost/optional.hpp>

#include <qubus/util/hpx/serialization/optional.hpp>

#include <chrono>
#include <memory>

namespace qubus
{

class vpu
{
public:
    vpu() = default;

    virtual ~vpu() = default;

    [[nodiscard]] virtual hpx::future<void> execute(const symbol_id& func,
                                                    execution_context ctx) = 0;
    [[nodiscard]] virtual hpx::future<boost::optional<performance_estimate>>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const = 0;

protected:
    vpu(const vpu&) = default;

    vpu& operator=(const vpu&) = default;

    vpu(vpu&&) = default;

    vpu& operator=(vpu&&) = default;
};

class remote_vpu_server : public hpx::components::component_base<remote_vpu_server>
{
public:
    remote_vpu_server() = default;

    explicit remote_vpu_server(std::unique_ptr<vpu> underlying_vpu_);

    void execute(const symbol_id& func, execution_context ctx);
    boost::optional<performance_estimate>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const;

    HPX_DEFINE_COMPONENT_ACTION(remote_vpu_server, execute, execute_action);
    HPX_DEFINE_COMPONENT_ACTION(remote_vpu_server, try_estimate_execution_time,
                                try_estimate_execution_time_action);

private:
    std::unique_ptr<vpu> underlying_vpu_;
};

class remote_vpu : public vpu, public hpx::components::client_base<remote_vpu, remote_vpu_server>
{
public:
    using base_type = hpx::components::client_base<remote_vpu, remote_vpu_server>;

    remote_vpu() = default;

    remote_vpu(hpx::id_type id);
    remote_vpu(hpx::future<hpx::id_type>&& id);

    [[nodiscard]] hpx::future<void> execute(const symbol_id& func, execution_context ctx) override;
    [[nodiscard]] hpx::future<boost::optional<performance_estimate>>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const override;
};

class remote_vpu_reference_server
: public hpx::components::component_base<remote_vpu_reference_server>
{
public:
    remote_vpu_reference_server() = default;

    explicit remote_vpu_reference_server(vpu* underlying_vpu_);

    void execute(const symbol_id& func, execution_context ctx);
    boost::optional<performance_estimate>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const;

    HPX_DEFINE_COMPONENT_ACTION(remote_vpu_reference_server, execute, execute_action);
    HPX_DEFINE_COMPONENT_ACTION(remote_vpu_reference_server, try_estimate_execution_time,
                                try_estimate_execution_time_action);

private:
    vpu* underlying_vpu_;
};

class remote_vpu_reference
: public vpu,
  public hpx::components::client_base<remote_vpu_reference, remote_vpu_reference_server>
{
public:
    using base_type =
        hpx::components::client_base<remote_vpu_reference, remote_vpu_reference_server>;

    remote_vpu_reference() = default;

    remote_vpu_reference(hpx::id_type id);
    remote_vpu_reference(hpx::future<hpx::id_type>&& id);

    [[nodiscard]] hpx::future<void> execute(const symbol_id& func, execution_context ctx) override;
    [[nodiscard]] hpx::future<boost::optional<performance_estimate>>
    try_estimate_execution_time(const symbol_id& func, const execution_context& ctx) const override;
};
} // namespace qubus

#endif
