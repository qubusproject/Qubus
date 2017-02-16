#include <qbb/qubus/jit/control_flow.hpp>

#include <qbb/qubus/jit/load_store.hpp>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>

inline namespace qbb
{
namespace qubus
{
namespace jit
{
void emit_if_else(reference condition, std::function<void()> then_emitter,
                  std::function<void()> else_emitter, llvm_environment& env,
                  compilation_context& ctx)
{
    auto& builder_ = env.builder();

    auto condition_value = load_from_ref(condition, env, ctx);

    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(env.ctx(), "then",
                                                            builder_.GetInsertBlock()->getParent());
    llvm::BasicBlock* else_block = llvm::BasicBlock::Create(env.ctx(), "else",
                                                            builder_.GetInsertBlock()->getParent());
    llvm::BasicBlock* merge = llvm::BasicBlock::Create(env.ctx(), "merge",
                                                       builder_.GetInsertBlock()->getParent());

    builder_.CreateCondBr(condition_value, then_block, else_block);

    builder_.SetInsertPoint(then_block);

    then_emitter();

    builder_.CreateBr(merge);

    builder_.SetInsertPoint(else_block);

    else_emitter();

    builder_.CreateBr(merge);

    builder_.SetInsertPoint(merge);
}
}
}
}