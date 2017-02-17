#ifndef QUBUS_JIT_LOAD_STORE_HPP
#define QUBUS_JIT_LOAD_STORE_HPP

#include <hpx/config.hpp>

#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/compilation_context.hpp>
#include <qubus/jit/reference.hpp>

#include <llvm/IR/Instructions.h>

namespace qubus
{
namespace jit
{
llvm::LoadInst* load_from_ref(const reference& ref, llvm_environment& env,
                              compilation_context& ctx);

llvm::StoreInst* store_to_ref(const reference& ref, llvm::Value* value, llvm_environment& env,
                              compilation_context& ctx);
}
}

#endif
