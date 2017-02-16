#ifndef QBB_QUBUS_JIT_CONTROL_FLOW_HPP
#define QBB_QUBUS_JIT_CONTROL_FLOW_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/jit/llvm_environment.hpp>
#include <qbb/qubus/jit/compilation_context.hpp>
#include <qbb/qubus/jit/reference.hpp>

#include <functional>

inline namespace qbb
{
namespace qubus
{
namespace jit
{
void emit_if_else(reference condition, std::function<void()> then_emitter,
                  std::function<void()> else_emitter, llvm_environment& env,
                  compilation_context& ctx);
}
}
}

#endif
