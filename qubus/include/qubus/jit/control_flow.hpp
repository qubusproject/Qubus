#ifndef QUBUS_JIT_CONTROL_FLOW_HPP
#define QUBUS_JIT_CONTROL_FLOW_HPP

#include <hpx/config.hpp>

#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/compilation_context.hpp>
#include <qubus/jit/reference.hpp>

#include <functional>

namespace qubus
{
namespace jit
{
void emit_if_else(reference condition, std::function<void()> then_emitter,
                  std::function<void()> else_emitter, llvm_environment& env,
                  compilation_context& ctx);
}
}

#endif
