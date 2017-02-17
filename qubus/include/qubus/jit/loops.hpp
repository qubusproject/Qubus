#ifndef QUBUS_JIT_LOOPS_HPP
#define QUBUS_JIT_LOOPS_HPP

#include <hpx/config.hpp>

#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/compilation_context.hpp>
#include <qubus/jit/reference.hpp>

#include <llvm/IR/Value.h>

#include <functional>

namespace qubus
{
namespace jit
{
void emit_loop(reference induction_variable, llvm::Value* lower_bound, llvm::Value* upper_bound,
               llvm::Value* increment, std::function<void()> body_emitter, llvm_environment& env,
               compilation_context& ctx);
}
}

#endif
