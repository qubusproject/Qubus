#ifndef QUBUS_CPU_COMPILER_HPP
#define QUBUS_CPU_COMPILER_HPP

#include <hpx/config.hpp>

#include <qubus/IR/module.hpp>
#include <qubus/IR/symbol_id.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace qubus
{

class cpu_runtime;

class cpu_plan
{
public:
    cpu_plan() = default;
    virtual ~cpu_plan() = default;

    cpu_plan(const cpu_plan&) = delete;
    cpu_plan& operator=(const cpu_plan&) = delete;

    virtual void execute(const symbol_id& entry_point, const std::vector<void*>& args,
                         cpu_runtime& runtime) const = 0;
};

class cpu_compiler_impl;

class cpu_compiler
{
public:
    cpu_compiler();
    cpu_compiler(const cpu_compiler&) = delete;
    cpu_compiler(cpu_compiler&&) = delete;

    cpu_compiler& operator=(const cpu_compiler&) = delete;
    cpu_compiler& operator=(cpu_compiler&&) = delete;

    std::unique_ptr<cpu_plan> compile_computelet(std::unique_ptr<module> program);

private:
    std::shared_ptr<cpu_compiler_impl> impl_;
};
}

#endif
