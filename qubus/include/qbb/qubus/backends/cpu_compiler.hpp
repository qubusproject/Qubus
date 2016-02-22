#ifndef QBB_QUBUS_CPU_COMPILER_HPP
#define QBB_QUBUS_CPU_COMPILER_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/backends/cpu_plan_registry.hpp>

#include <qbb/qubus/IR/function_declaration.hpp>

#include <qbb/qubus/compiler.hpp>
#include <qbb/qubus/plan.hpp>

#include <memory>

namespace qbb
{
namespace qubus
{
class cpu_compiler_impl;

class cpu_compiler : public compiler
{
public:
    explicit cpu_compiler(cpu_plan_registry& plan_registry_);

    cpu_compiler(const cpu_compiler&) = delete;
    cpu_compiler(cpu_compiler&&) = delete;

    cpu_compiler& operator=(const cpu_compiler&) = delete;
    cpu_compiler& operator=(cpu_compiler&&) = delete;

    virtual ~cpu_compiler() = default;

    plan compile_plan(const function_declaration& plan_decl) override;

private:
    std::shared_ptr<cpu_compiler_impl> impl_;
};
}
}

#endif
