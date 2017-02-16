#ifndef QUBUS_JIT_ENTRY_BLOCK_ALLOCA_HPP
#define QUBUS_JIT_ENTRY_BLOCK_ALLOCA_HPP

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/ADT/Twine.h>

namespace qubus
{
namespace jit
{
llvm::AllocaInst *create_entry_block_alloca(llvm::Function *current_function, llvm::Type *type,
                                            llvm::Value *array_size = 0,
                                            const llvm::Twine &name = "");
}
}

#endif
