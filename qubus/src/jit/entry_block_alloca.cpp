#include <qbb/qubus/jit/entry_block_alloca.hpp>

#include <llvm/IR/IRBuilder.h>

namespace qubus
{
namespace jit
{
llvm::AllocaInst* create_entry_block_alloca(llvm::Function* current_function, llvm::Type* type,
                                            llvm::Value* array_size,
                                            const llvm::Twine& name)
{
    llvm::IRBuilder<> builder(&current_function->getEntryBlock(),
                              current_function->getEntryBlock().begin());
    return builder.CreateAlloca(type, array_size, name);
}
}
}