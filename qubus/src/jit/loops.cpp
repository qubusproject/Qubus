#include <qbb/qubus/jit/loops.hpp>

#include <qbb/qubus/jit/load_store.hpp>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>

namespace qbb
{
namespace qubus
{
namespace jit
{

void emit_loop(reference induction_variable, llvm::Value* lower_bound, llvm::Value* upper_bound,
               llvm::Value* increment, std::function<void()> body_emitter, llvm_environment& env,
               compilation_context& ctx)
{
    auto& builder_ = env.builder();

    store_to_ref(induction_variable, lower_bound, env, ctx);

    llvm::BasicBlock* header = llvm::BasicBlock::Create(env.ctx(), "header",
                                                        builder_.GetInsertBlock()->getParent());
    llvm::BasicBlock* body = llvm::BasicBlock::Create(env.ctx(), "body",
                                                      builder_.GetInsertBlock()->getParent());
    llvm::BasicBlock* exit = llvm::BasicBlock::Create(env.ctx(), "exit",
                                                      builder_.GetInsertBlock()->getParent());

    builder_.CreateBr(header);

    builder_.SetInsertPoint(header);

    auto induction_variable_value = load_from_ref(induction_variable, env, ctx);

    llvm::Value* exit_cond = builder_.CreateICmpSLT(induction_variable_value, upper_bound);

    builder_.CreateCondBr(exit_cond, body, exit);

    builder_.SetInsertPoint(body);

    body_emitter();

    auto induction_variable_value2 = load_from_ref(induction_variable, env, ctx);

    store_to_ref(induction_variable,
                 builder_.CreateAdd(induction_variable_value2, increment, "", true, true), env,
                 ctx);

    builder_.CreateBr(header);

    body = builder_.GetInsertBlock();

    exit->moveAfter(body);

    builder_.SetInsertPoint(exit);
}
}
}
}