#ifndef QUBUS_CPU_COMPILER_HPP
#define QUBUS_CPU_COMPILER_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/function_declaration.hpp>

#include <memory>
#include <functional>
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

    virtual void execute(const std::vector<void*>& args, cpu_runtime& runtime) const = 0;
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

    std::unique_ptr<cpu_plan> compile_computelet(const function_declaration& computelet);
private:
    std::shared_ptr<cpu_compiler_impl> impl_;
};
}

#endif
