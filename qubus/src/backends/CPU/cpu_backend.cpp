#include <hpx/config.hpp>

#include <qbb/qubus/backends/cpu_backend.hpp>

#include <qbb/qubus/backend.hpp>

#include <qbb/qubus/backends/llvm_environment.hpp>
#include <qbb/qubus/backends/cpuinfo.hpp>
#include <qbb/qubus/backends/local_array_alias_analysis.hpp>
#include <qbb/qubus/backends/reference.hpp>
#include <qbb/qubus/backends/alias_info.hpp>

#include <qbb/qubus/backends/cpu_allocator.hpp>
#include <qbb/qubus/backends/cpu_memory_block.hpp>
#include <qbb/qubus/local_address_space.hpp>

#include <qbb/qubus/backends/cpu_object_factory.hpp>

#include <qbb/qubus/abi_info.hpp>

#include <qbb/qubus/backends/execution_stack.hpp>
#include <qbb/qubus/metadata_builder.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <qbb/qubus/IR/type_inference.hpp>

#include <qbb/util/make_unique.hpp>

#include <qubus/qbb_qubus_export.h>

#include <hpx/async.hpp>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/JITEventListener.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <llvm/PassManager.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DataLayout.h>

#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Vectorize.h>

#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>

#include <hpx/lcos/local/promise.hpp>
#include <hpx/lcos/future.hpp>
#include <hpx/lcos/wait_all.hpp>

#include <boost/optional.hpp>
#include <boost/signals2.hpp>

#include <qbb/util/optional_ref.hpp>
#include <qbb/util/make_unique.hpp>
#include <qbb/util/assert.hpp>
#include <qbb/util/unused.hpp>

#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <algorithm>
#include <vector>

namespace qbb
{
namespace qubus
{

class cpu_runtime
{
public:
    cpu_runtime() : scratch_mem_(8 * 1024 * 1024), current_stack_ptr_(scratch_mem_.data())
    {
    }

    void* alloc_scratch_mem(util::index_t size)
    {
        void* addr = current_stack_ptr_;

        current_stack_ptr_ += size;

        return addr;
    }

    void dealloc_scratch_mem(util::index_t size)
    {
        current_stack_ptr_ -= size;
    }

private:
    std::vector<char> scratch_mem_;
    char* current_stack_ptr_;
};

extern "C" QBB_QUBUS_EXPORT void* qbb_qubus_cpurt_alloc_scatch_mem(cpu_runtime* runtime,
                                                                   util::index_t size)
{
    return runtime->alloc_scratch_mem(size);
}

extern "C" QBB_QUBUS_EXPORT void qbb_qubus_cpurt_dealloc_scratch_mem(cpu_runtime* runtime,
                                                                     util::index_t size)
{
    runtime->dealloc_scratch_mem(size);
}

namespace
{

class code_region
{
public:
    code_region(util::handle token_, const expression& QBB_UNUSED(expr_), llvm_environment& env_)
    : token_(token_), laa_alias_analysis_(token_, env_)
    {
    }

    code_region(const code_region&) = delete;
    code_region& operator=(const code_region&) = delete;

    alias_info register_access(variable_declaration accessed_array, std::vector<expression> indices,
                               reference data_ref)
    {
        return laa_alias_analysis_.query(accessed_array, indices, data_ref);
    }

private:
    util::handle token_;

    local_array_access_alias_analysis laa_alias_analysis_;
};

llvm::AllocaInst* create_entry_block_alloca(llvm::Function* current_function, llvm::Type* type,
                                            llvm::Value* array_size = 0,
                                            const llvm::Twine& name = "")
{
    llvm::IRBuilder<> builder(&current_function->getEntryBlock(),
                              current_function->getEntryBlock().begin());
    return builder.CreateAlloca(type, array_size, name);
}

class scope
{
public:
    using on_scope_exit_signal = boost::signals2::signal<void()>;

    scope() : on_scope_exit_(util::make_unique<on_scope_exit_signal>())
    {
    }

    scope(const scope&) = delete;
    scope& operator=(const scope&) = delete;

    scope(scope&&) = default;
    scope& operator=(scope&&) = default;

    ~scope()
    {
        if (on_scope_exit_)
        {
            (*on_scope_exit_)();
        }
    }

    void on_exit(const on_scope_exit_signal::slot_type& subscriber)
    {
        on_scope_exit_->connect(subscriber);
    }

private:
    std::unique_ptr<on_scope_exit_signal> on_scope_exit_;
};

class global_alias_info_query
{
public:
    global_alias_info_query(reference ref_) : ref_(std::move(ref_))
    {
    }

    global_alias_info_query(const global_alias_info_query&) = delete;
    global_alias_info_query& operator=(const global_alias_info_query&) = delete;

    global_alias_info_query(global_alias_info_query&&) = default;
    global_alias_info_query& operator=(global_alias_info_query&&) = default;

    const reference& ref() const
    {
        return ref_;
    }

    void answer(llvm::MDNode* alias_scope, llvm::MDNode* alias_set)
    {
        alias_scope_promise_.set_value(alias_scope);
        alias_set_promise_.set_value(alias_set);
    }

    hpx::lcos::shared_future<llvm::MDNode*> get_alias_scope()
    {
        return alias_scope_promise_.get_future();
    }

    hpx::lcos::shared_future<llvm::MDNode*> get_alias_set()
    {
        return alias_set_promise_.get_future();
    }

private:
    reference ref_;
    hpx::lcos::local::promise<llvm::MDNode*> alias_scope_promise_;
    hpx::lcos::local::promise<llvm::MDNode*> alias_set_promise_;
};

class compilation_context
{
public:
    compilation_context(llvm_environment& env_) : env_(&env_), scopes_(1), next_region_token_(0)
    {
    }

    ~compilation_context()
    {
        answer_pending_global_alias_queries();

        wait_on_pending_tasks();
    }

    compilation_context(const compilation_context&) = delete;
    compilation_context& operator=(const compilation_context&) = delete;

    std::map<qbb::util::handle, reference>& symbol_table()
    {
        return symbol_table_;
    }

    const std::map<qbb::util::handle, reference>& symbol_table() const
    {
        return symbol_table_;
    }

    boost::optional<function_declaration> get_next_plan_to_compile()
    {
        if (plans_to_compile_.empty())
            return boost::none;

        auto next_plan_to_compile = plans_to_compile_.back();

        plans_to_compile_.pop_back();

        return next_plan_to_compile;
    }

    void add_plan_to_compile(function_declaration fn)
    {
        plans_to_compile_.push_back(std::move(fn));
    }

    scope& enter_new_scope()
    {
        scopes_.emplace_back();

        return scopes_.back();
    }

    scope& get_current_scope()
    {
        return scopes_.back();
    }

    const scope& get_current_scope() const
    {
        return scopes_.back();
    }

    void exit_current_scope()
    {
        scopes_.pop_back();
    }

    void enter_code_region(const expression& expr)
    {
        current_code_region_ =
            util::make_unique<code_region>(util::handle(next_region_token_), expr, *env_);
        next_region_token_++;
    }

    void leave_code_region()
    {
        current_code_region_.reset();
    }

    util::optional_ref<code_region> current_code_region()
    {
        return current_code_region_ ? util::optional_ref<code_region>(*current_code_region_)
                                    : util::optional_ref<code_region>();
    }

    alias_info query_global_alias_info(const reference& ref) const
    {
        pending_global_alias_queries_.emplace_back(ref);

        auto& query = pending_global_alias_queries_.back();

        return alias_info(query.get_alias_scope(), query.get_alias_set());
    }

    void register_pending_task(hpx::lcos::future<void> f)
    {
        pending_tasks_.push_back(std::move(f));
    }

    void wait_on_pending_tasks() const
    {
        hpx::wait_all(pending_tasks_);
    }

private:
    void answer_pending_global_alias_queries()
    {
        std::map<std::string, llvm::MDNode*> alias_scope_table;

        llvm::MDNode* global_alias_domain =
            env_->md_builder().createAliasScopeDomain("qubus.alias_domain");

        for (const auto& query : pending_global_alias_queries_)
        {
            std::string name = query.ref().origin().str();

            alias_scope_table[name] =
                env_->md_builder().createAliasScope(name, global_alias_domain);
        }

        for (auto& query : pending_global_alias_queries_)
        {
            std::string name = query.ref().origin().str();

            std::vector<llvm::Metadata*> alias_scopes;

            for (const auto& entry : alias_scope_table)
            {
                if (entry.first != name)
                {
                    alias_scopes.push_back(entry.second);
                }
            }

            auto noalias_set = llvm::MDNode::get(llvm::getGlobalContext(), alias_scopes);

            auto alias_scope = alias_scope_table.at(name);

            query.answer(alias_scope, noalias_set);
        }
    }

    llvm_environment* env_;

    std::map<qbb::util::handle, reference> symbol_table_;
    std::vector<function_declaration> plans_to_compile_;
    std::vector<scope> scopes_;

    std::unique_ptr<code_region> current_code_region_;
    std::uintptr_t next_region_token_;

    mutable std::vector<global_alias_info_query> pending_global_alias_queries_;

    std::vector<hpx::lcos::future<void>> pending_tasks_;
};

llvm::LoadInst* load_from_ref(const reference& ref, llvm_environment& env, compilation_context& ctx)
{
    auto& builder = env.builder();

    auto value = builder.CreateLoad(ref.addr());

    llvm::MDNode* alias_scopes = llvm::MDNode::get(llvm::getGlobalContext(), {});
    llvm::MDNode* noalias_set = llvm::MDNode::get(llvm::getGlobalContext(), {});
    value->setMetadata("alias.scope", alias_scopes);
    value->setMetadata("noalias", noalias_set);

    auto alias_info = ctx.query_global_alias_info(ref);
    ref.add_alias_info(alias_info);

    for (const auto& entry : ref.alias_scopes())
    {
        auto f =
            entry.then(hpx::launch::sync_policies,
                       [value](const hpx::lcos::shared_future<llvm::MDNode*>& alias_scope)
                       {
                           auto alias_scopes = value->getMetadata("alias.scope");

                           std::vector<llvm::Metadata*> scopes = {alias_scope.get()};
                           auto result = llvm::MDNode::concatenate(
                               alias_scopes, llvm::MDNode::get(llvm::getGlobalContext(), scopes));

                           value->setMetadata("alias.scope", result);
                       });

        ctx.register_pending_task(std::move(f));
    }

    for (const auto& entry : ref.noalias_sets())
    {
        auto f = entry.then(hpx::launch::sync_policies,
                            [value](const hpx::lcos::shared_future<llvm::MDNode*>& noalias_set)
                            {
                                auto noalias_sets = value->getMetadata("noalias");

                                auto result =
                                    llvm::MDNode::concatenate(noalias_sets, noalias_set.get());

                                value->setMetadata("noalias", result);
                            });

        ctx.register_pending_task(std::move(f));
    }

    return value;
}

llvm::StoreInst* store_to_ref(const reference& ref, llvm::Value* value, llvm_environment& env,
                              compilation_context& ctx)
{
    auto& builder = env.builder();

    auto store = builder.CreateStore(value, ref.addr());

    llvm::MDNode* alias_scopes = llvm::MDNode::get(llvm::getGlobalContext(), {});
    llvm::MDNode* noalias_set = llvm::MDNode::get(llvm::getGlobalContext(), {});
    store->setMetadata("alias.scope", alias_scopes);
    store->setMetadata("noalias", noalias_set);

    auto alias_info = ctx.query_global_alias_info(ref);
    ref.add_alias_info(alias_info);

    for (const auto& entry : ref.alias_scopes())
    {
        auto f =
            entry.then([store](const hpx::lcos::shared_future<llvm::MDNode*>& alias_scope)
                       {
                           auto alias_scopes = store->getMetadata("alias.scope");

                           std::vector<llvm::Metadata*> scopes = {alias_scope.get()};
                           auto result = llvm::MDNode::concatenate(
                               alias_scopes, llvm::MDNode::get(llvm::getGlobalContext(), scopes));

                           store->setMetadata("alias.scope", result);
                       });

        ctx.register_pending_task(std::move(f));
    }

    for (const auto& entry : ref.noalias_sets())
    {
        auto f = entry.then([store](const hpx::lcos::shared_future<llvm::MDNode*>& noalias_set)
                            {
                                auto noalias_sets = store->getMetadata("noalias");

                                auto result =
                                    llvm::MDNode::concatenate(noalias_sets, noalias_set.get());

                                store->setMetadata("noalias", result);
                            });

        ctx.register_pending_task(std::move(f));
    }

    return store;
}

reference compile(const expression& expr, llvm_environment& env, compilation_context& ctx);

template <typename BodyEmitter>
void emit_loop(reference induction_variable, llvm::Value* lower_bound, llvm::Value* upper_bound,
               llvm::Value* increment, BodyEmitter body_emitter, llvm_environment& env,
               compilation_context& ctx)
{
    auto& builder_ = env.builder();

    store_to_ref(induction_variable, lower_bound, env, ctx);

    llvm::BasicBlock* header = llvm::BasicBlock::Create(llvm::getGlobalContext(), "header",
                                                        builder_.GetInsertBlock()->getParent());
    llvm::BasicBlock* body = llvm::BasicBlock::Create(llvm::getGlobalContext(), "body",
                                                      builder_.GetInsertBlock()->getParent());
    llvm::BasicBlock* exit = llvm::BasicBlock::Create(llvm::getGlobalContext(), "exit",
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

template <typename ThenEmitter, typename ElseEmitter>
void emit_if_else(reference condition, ThenEmitter then_emitter, ElseEmitter else_emitter,
                  llvm_environment& env, compilation_context& ctx)
{
    auto& builder_ = env.builder();

    auto condition_value = load_from_ref(condition, env, ctx);

    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then",
                                                            builder_.GetInsertBlock()->getParent());
    llvm::BasicBlock* else_block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else",
                                                            builder_.GetInsertBlock()->getParent());
    llvm::BasicBlock* merge = llvm::BasicBlock::Create(llvm::getGlobalContext(), "merge",
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

reference emit_binary_operator(binary_op_tag tag, const expression& left, const expression& right,
                               llvm_environment& env, compilation_context& ctx)
{
    using pattern::_;

    reference left_value_ptr = compile(left, env, ctx);
    reference right_value_ptr = compile(right, env, ctx);

    auto& builder = env.builder();

    llvm::Instruction* left_value = load_from_ref(left_value_ptr, env, ctx);

    llvm::Instruction* right_value = load_from_ref(right_value_ptr, env, ctx);

    type result_type = typeof_(left);

    llvm::Value* result;

    switch (tag)
    {
    case binary_op_tag::assign:
    {
        store_to_ref(left_value_ptr, right_value, env, ctx);

        return reference();
    }
    case binary_op_tag::plus_assign:
    {
        auto m =
            pattern::make_matcher<type, llvm::Value*>()
                .case_(pattern::float_t || pattern::double_t,
                       [&]
                       {
                           return builder.CreateFAdd(left_value, right_value);
                       })
                .case_(pattern::integer_t,
                       [&]
                       {
                           return builder.CreateAdd(left_value, right_value);
                       })
                .case_(pattern::complex_t(_), [&]
                       {
                           llvm::Value* lhs_real =
                               builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 0});
                           llvm::Value* lhs_imag =
                               builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 1});

                           llvm::Value* rhs_real =
                               builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 0});
                           llvm::Value* rhs_imag =
                               builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 1});

                           llvm::Value* result = llvm::UndefValue::get(left_value->getType());

                           llvm::Value* result_real = builder.CreateFAdd(lhs_real, rhs_real);
                           llvm::Value* result_imag = builder.CreateFAdd(lhs_imag, rhs_imag);

                           result = builder.CreateInsertValue(result, result_real,
                                                              std::vector<unsigned>{0, 0});
                           result = builder.CreateInsertValue(result, result_imag,
                                                              std::vector<unsigned>{0, 1});

                           return result;
                       });

        llvm::Value* sum = pattern::match(result_type, m);

        store_to_ref(left_value_ptr, sum, env, ctx);

        return reference();
    }
    case binary_op_tag::plus:
    {
        auto m =
            pattern::make_matcher<type, llvm::Value*>()
                .case_(pattern::float_t || pattern::double_t,
                       [&]
                       {
                           return builder.CreateFAdd(left_value, right_value);
                       })
                .case_(pattern::integer_t,
                       [&]
                       {
                           return builder.CreateAdd(left_value, right_value, "", true, true);
                       })
                .case_(pattern::complex_t(_), [&]
                       {
                           llvm::Value* lhs_real =
                               builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 0});
                           llvm::Value* lhs_imag =
                               builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 1});

                           llvm::Value* rhs_real =
                               builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 0});
                           llvm::Value* rhs_imag =
                               builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 1});

                           llvm::Value* result = llvm::UndefValue::get(left_value->getType());

                           llvm::Value* result_real = builder.CreateFAdd(lhs_real, rhs_real);
                           llvm::Value* result_imag = builder.CreateFAdd(lhs_imag, rhs_imag);

                           result = builder.CreateInsertValue(result, result_real,
                                                              std::vector<unsigned>{0, 0});
                           result = builder.CreateInsertValue(result, result_imag,
                                                              std::vector<unsigned>{0, 1});

                           return result;
                       });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::minus:
    {
        auto m =
            pattern::make_matcher<type, llvm::Value*>()
                .case_(pattern::float_t || pattern::double_t,
                       [&]
                       {
                           return builder.CreateFSub(left_value, right_value);
                       })
                .case_(pattern::integer_t,
                       [&]
                       {
                           return builder.CreateSub(left_value, right_value, "", true, true);
                       })
                .case_(pattern::complex_t(_), [&]
                       {
                           llvm::Value* lhs_real =
                               builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 0});
                           llvm::Value* lhs_imag =
                               builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 1});

                           llvm::Value* rhs_real =
                               builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 0});
                           llvm::Value* rhs_imag =
                               builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 1});

                           llvm::Value* result = llvm::UndefValue::get(left_value->getType());

                           llvm::Value* result_real = builder.CreateFSub(lhs_real, rhs_real);
                           llvm::Value* result_imag = builder.CreateFSub(lhs_imag, rhs_imag);

                           result = builder.CreateInsertValue(result, result_real,
                                                              std::vector<unsigned>{0, 0});
                           result = builder.CreateInsertValue(result, result_imag,
                                                              std::vector<unsigned>{0, 1});

                           return result;
                       });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::multiplies:
    {
        auto m =
            pattern::make_matcher<type, llvm::Value*>()
                .case_(pattern::float_t || pattern::double_t,
                       [&]
                       {
                           return builder.CreateFMul(left_value, right_value);
                       })
                .case_(pattern::integer_t,
                       [&]
                       {
                           return builder.CreateMul(left_value, right_value, "", true, true);
                       })
                .case_(pattern::complex_t(_), [&]
                       {
                           llvm::Value* lhs_real =
                               builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 0});
                           llvm::Value* lhs_imag =
                               builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 1});

                           llvm::Value* rhs_real =
                               builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 0});
                           llvm::Value* rhs_imag =
                               builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 1});

                           llvm::Value* result = llvm::UndefValue::get(left_value->getType());

                           llvm::Value* prod_rr = builder.CreateFMul(lhs_real, rhs_real);
                           llvm::Value* prod_ii = builder.CreateFMul(lhs_imag, rhs_imag);
                           llvm::Value* result_real = builder.CreateFSub(prod_rr, prod_ii);

                           llvm::Value* prod_ri = builder.CreateFMul(lhs_real, rhs_imag);
                           llvm::Value* prod_ir = builder.CreateFMul(lhs_imag, rhs_real);
                           llvm::Value* result_imag = builder.CreateFAdd(prod_ri, prod_ir);

                           result = builder.CreateInsertValue(result, result_real,
                                                              std::vector<unsigned>{0, 0});
                           result = builder.CreateInsertValue(result, result_imag,
                                                              std::vector<unsigned>{0, 1});

                           return result;
                       });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::divides:
    {
        auto m =
            pattern::make_matcher<type, llvm::Value*>()
                .case_(pattern::float_t || pattern::double_t,
                       [&]
                       {
                           return builder.CreateFDiv(left_value, right_value);
                       })
                .case_(pattern::integer_t,
                       [&]
                       {
                           return builder.CreateSDiv(left_value, right_value);
                       })
                .case_(pattern::complex_t(_), [&]
                       {
                           llvm::Value* lhs_real =
                               builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 0});
                           llvm::Value* lhs_imag =
                               builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 1});

                           llvm::Value* rhs_real =
                               builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 0});
                           llvm::Value* rhs_imag =
                               builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 1});

                           llvm::Value* result = llvm::UndefValue::get(left_value->getType());

                           llvm::Value* rhs_norm =
                               builder.CreateFAdd(builder.CreateFMul(rhs_real, rhs_real),
                                                  builder.CreateFMul(rhs_imag, rhs_imag));

                           llvm::Value* prod_rr = builder.CreateFMul(lhs_real, rhs_real);
                           llvm::Value* prod_ii = builder.CreateFMul(lhs_imag, rhs_imag);
                           llvm::Value* result_real = builder.CreateFAdd(prod_rr, prod_ii);

                           llvm::Value* prod_ri = builder.CreateFMul(lhs_real, rhs_imag);
                           llvm::Value* prod_ir = builder.CreateFMul(lhs_imag, rhs_real);
                           llvm::Value* result_imag = builder.CreateFSub(prod_ir, prod_ri);

                           result_real = builder.CreateFDiv(result_real, rhs_norm);
                           result_imag = builder.CreateFDiv(result_imag, rhs_norm);

                           result = builder.CreateInsertValue(result, result_real,
                                                              std::vector<unsigned>{0, 0});
                           result = builder.CreateInsertValue(result, result_imag,
                                                              std::vector<unsigned>{0, 1});

                           return result;
                       });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::modulus:
    {
        auto m = pattern::make_matcher<type, llvm::Value*>()
                     .case_(pattern::float_t || pattern::double_t,
                            [&]
                            {
                                return builder.CreateFRem(left_value, right_value);
                            })
                     .case_(pattern::integer_t, [&]
                            {
                                return builder.CreateSRem(left_value, right_value);
                            });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::div_floor:
    {
        auto m = pattern::make_matcher<type, llvm::Value*>().case_(
            pattern::integer_t, [&]
            {
                llvm::Type* integer_type = env.map_qubus_type(types::integer{});

                auto quotient = builder.CreateSDiv(left_value, right_value);
                auto remainder = builder.CreateSRem(left_value, right_value);

                auto is_exact =
                    builder.CreateICmpEQ(remainder, llvm::ConstantInt::get(integer_type, 0, true));
                auto is_positive = builder.CreateICmpEQ(
                    builder.CreateICmpSLT(remainder, llvm::ConstantInt::get(integer_type, 0, true)),
                    builder.CreateICmpSLT(quotient, llvm::ConstantInt::get(integer_type, 0, true)));
                auto is_exact_or_positive = builder.CreateOr(is_exact, is_positive);

                auto reduced_quotient =
                    builder.CreateSub(quotient, llvm::ConstantInt::get(integer_type, 1, true));

                return builder.CreateSelect(is_exact_or_positive, quotient, reduced_quotient);
            });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::equal_to:
    {
        auto m = pattern::make_matcher<type, llvm::Value*>().case_(
            pattern::integer_t || pattern::bool_t, [&]
            {
                return builder.CreateICmpEQ(left_value, right_value);
            });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::less:
    {
        auto m = pattern::make_matcher<type, llvm::Value*>().case_(
            pattern::integer_t || pattern::bool_t, [&]
            {
                return builder.CreateICmpSLT(left_value, right_value);
            });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::less_equal:
    {
        auto m = pattern::make_matcher<type, llvm::Value*>().case_(
            pattern::integer_t || pattern::bool_t, [&]
            {
                return builder.CreateICmpSLE(left_value, right_value);
            });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::greater:
    {
        auto m = pattern::make_matcher<type, llvm::Value*>().case_(
            pattern::integer_t || pattern::bool_t, [&]
            {
                return builder.CreateICmpSGT(left_value, right_value);
            });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::greater_equal:
    {
        auto m = pattern::make_matcher<type, llvm::Value*>().case_(
            pattern::integer_t || pattern::bool_t, [&]
            {
                return builder.CreateICmpSGE(left_value, right_value);
            });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::logical_and:
    {
        auto m = pattern::make_matcher<type, llvm::Value*>().case_(pattern::bool_t, [&]
                                                                   {
                                                                       return builder.CreateAnd(
                                                                           left_value, right_value);
                                                                   });

        result = pattern::match(result_type, m);

        break;
    }
    case binary_op_tag::logical_or:
    {
        auto m = pattern::make_matcher<type, llvm::Value*>().case_(pattern::bool_t, [&]
                                                                   {
                                                                       return builder.CreateOr(
                                                                           left_value, right_value);
                                                                   });

        result = pattern::match(result_type, m);

        break;
    }
    default:
        throw 0;
    }

    auto result_var = create_entry_block_alloca(env.get_current_function(), result->getType());

    reference result_var_ref(result_var, access_path());
    store_to_ref(result_var_ref, result, env, ctx);

    return result_var_ref;
}

reference emit_unary_operator(unary_op_tag tag, const expression& arg, llvm_environment& env,
                              compilation_context& ctx)
{
    using pattern::_;

    reference arg_value_ptr = compile(arg, env, ctx);

    auto& builder = env.builder();

    llvm::Instruction* arg_value = load_from_ref(arg_value_ptr, env, ctx);

    type result_type = typeof_(arg);

    llvm::Value* result;

    switch (tag)
    {
    case unary_op_tag::negate:
    {
        auto m =
            pattern::make_matcher<type, llvm::Value*>()
                .case_(pattern::float_t || pattern::double_t,
                       [&]
                       {
                           return builder.CreateFNeg(arg_value);
                       })
                .case_(pattern::integer_t,
                       [&]
                       {
                           return builder.CreateNeg(arg_value);
                       })
                .case_(pattern::complex_t(_), [&]
                       {
                           llvm::Value* real =
                               builder.CreateExtractValue(arg_value, std::vector<unsigned>{0, 0});
                           llvm::Value* imag =
                               builder.CreateExtractValue(arg_value, std::vector<unsigned>{0, 1});

                           llvm::Value* result = llvm::UndefValue::get(arg_value->getType());

                           llvm::Value* result_real = builder.CreateFNeg(real);
                           llvm::Value* result_imag = builder.CreateFNeg(imag);

                           result = builder.CreateInsertValue(result, result_real,
                                                              std::vector<unsigned>{0, 0});
                           result = builder.CreateInsertValue(result, result_imag,
                                                              std::vector<unsigned>{0, 1});

                           return result;
                       });

        result = pattern::match(result_type, m);

        break;
    }
    case unary_op_tag::plus:
        result = arg_value;
        break;
    default:
        throw 0;
    }

    auto result_var = create_entry_block_alloca(env.get_current_function(), result->getType());

    reference result_var_ref(result_var, access_path());

    store_to_ref(result_var_ref, result, env, ctx);

    return result_var_ref;
}

reference emit_type_conversion(const type& target_type, const expression& arg,
                               llvm_environment& env, compilation_context& ctx)
{
    using pattern::_;

    reference arg_value_ptr = compile(arg, env, ctx);

    auto& builder = env.builder();

    llvm::Instruction* arg_value = load_from_ref(arg_value_ptr, env, ctx);

    type arg_type = typeof_(arg);

    auto m =
        pattern::make_matcher<type, llvm::Value*>()
            .case_(pattern::double_t,
                   [&]
                   {
                       auto m2 = pattern::make_matcher<type, llvm::Value*>()
                                     .case_(complex_t(pattern::double_t),
                                            [&]
                                            {
                                                llvm::Type* complex_type = env.map_qubus_type(
                                                    types::complex(types::double_()));

                                                llvm::Value* result =
                                                    llvm::ConstantAggregateZero::get(complex_type);

                                                result = builder.CreateInsertValue(
                                                    result, arg_value, std::vector<unsigned>{0, 0});

                                                return result;
                                            })
                                     .case_(pattern::double_t, [&]
                                            {
                                                return arg_value;
                                            });

                       return pattern::match(target_type, m2);
                   })
            .case_(pattern::float_t,
                   [&]
                   {
                       auto m2 =
                           pattern::make_matcher<type, llvm::Value*>()
                               .case_(complex_t(pattern::double_t),
                                      [&]
                                      {
                                          llvm::Type* complex_type =
                                              env.map_qubus_type(types::complex(types::double_()));

                                          llvm::Value* result =
                                              llvm::ConstantAggregateZero::get(complex_type);

                                          result = builder.CreateInsertValue(
                                              result,
                                              builder.CreateFPExt(
                                                  arg_value, env.map_qubus_type(types::double_())),
                                              std::vector<unsigned>{0, 0});

                                          return result;
                                      })
                               .case_(complex_t(pattern::float_t),
                                      [&]
                                      {
                                          llvm::Type* complex_type =
                                              env.map_qubus_type(types::complex(types::float_()));

                                          llvm::Value* result =
                                              llvm::ConstantAggregateZero::get(complex_type);

                                          result = builder.CreateInsertValue(
                                              result, arg_value, std::vector<unsigned>{0, 0});

                                          return result;
                                      })
                               .case_(pattern::double_t,
                                      [&]
                                      {
                                          return builder.CreateFPExt(
                                              arg_value, env.map_qubus_type(target_type));
                                      })
                               .case_(pattern::float_t, [&]
                                      {
                                          return arg_value;
                                      });

                       return pattern::match(target_type, m2);
                   })
            .case_(pattern::integer_t,
                   [&]
                   {
                       pattern::variable<type> subtype;

                       auto m2 = pattern::make_matcher<type, llvm::Value*>()
                                     .case_(complex_t(subtype),
                                            [&]
                                            {
                                                llvm::Type* complex_type =
                                                    env.map_qubus_type(target_type);

                                                llvm::Value* result =
                                                    llvm::ConstantAggregateZero::get(complex_type);

                                                result = builder.CreateInsertValue(
                                                    result, builder.CreateSIToFP(
                                                                arg_value,
                                                                env.map_qubus_type(subtype.get())),
                                                    std::vector<unsigned>{0, 0});

                                                return result;
                                            })
                                     .case_(bind_to(pattern::float_t || pattern::double_t, subtype),
                                            [&]
                                            {
                                                return builder.CreateSIToFP(
                                                    arg_value, env.map_qubus_type(subtype.get()));
                                            })
                                     .case_(pattern::integer_t, [&]
                                            {
                                                return arg_value;
                                            });

                       return pattern::match(target_type, m2);
                   })
            .case_(complex_t(pattern::float_t),
                   [&]
                   {
                       auto m2 =
                           pattern::make_matcher<type, llvm::Value*>()
                               .case_(complex_t(pattern::double_t),
                                      [&]
                                      {
                                          llvm::Type* complex_type =
                                              env.map_qubus_type(target_type);

                                          llvm::Value* result = llvm::UndefValue::get(complex_type);

                                          llvm::Value* arg_value_real = builder.CreateExtractValue(
                                              arg_value, std::vector<unsigned>{0, 0});
                                          llvm::Value* arg_value_imag = builder.CreateExtractValue(
                                              arg_value, std::vector<unsigned>{0, 1});

                                          result = builder.CreateInsertValue(
                                              result, builder.CreateFPExt(
                                                          arg_value_real,
                                                          env.map_qubus_type(types::double_())),
                                              std::vector<unsigned>{0, 0});

                                          result = builder.CreateInsertValue(
                                              result, builder.CreateFPExt(
                                                          arg_value_imag,
                                                          env.map_qubus_type(types::double_())),
                                              std::vector<unsigned>{0, 1});

                                          return result;
                                      })
                               .case_(complex_t(pattern::float_t), [&]
                                      {
                                          return arg_value;
                                      });

                       return pattern::match(target_type, m2);
                   })
            .case_(complex_t(pattern::double_t), [&]
                   {
                       auto m2 = pattern::make_matcher<type, llvm::Value*>().case_(
                           complex_t(pattern::double_t), [&]
                           {
                               return arg_value;
                           });

                       return pattern::match(target_type, m2);
                   });

    llvm::Value* result = pattern::match(arg_type, m);

    auto result_var = create_entry_block_alloca(env.get_current_function(), result->getType());

    reference result_var_ref(result_var, access_path());

    store_to_ref(result_var_ref, result, env, ctx);

    return result_var_ref;
}

llvm::Value* emit_array_access(llvm::Value* data, const std::vector<llvm::Value*>& shape,
                               const std::vector<llvm::Value*>& indices, llvm_environment& env)
{
    auto& builder = env.builder();

    llvm::Type* size_type = env.map_qubus_type(types::integer());

    llvm::Value* linearized_index = llvm::ConstantInt::get(size_type, 0);

    // for (std::size_t i = indices.size(); i-- > 0;)
    for (std::size_t i = 0; i < indices.size(); i++)
    {
        auto extent = shape[i];

        linearized_index =
            builder.CreateAdd(builder.CreateMul(extent, linearized_index, "idx_mul", true, true),
                              indices[i], "idx_add", true, true);
    }

    return builder.CreateInBoundsGEP(data, linearized_index);
}

reference emit_array_access(const reference& data, const reference& shape,
                            const std::vector<llvm::Value*>& indices, llvm_environment& env,
                            compilation_context& ctx)
{
    auto& builder = env.builder();

    std::vector<llvm::Value*> shape_;
    shape_.reserve(indices.size());

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        auto extent_ptr = builder.CreateConstInBoundsGEP1_32(shape.addr(), i);

        auto extent = load_from_ref(reference(extent_ptr, shape.origin()), env, ctx);

        shape_.push_back(extent);
    }

    auto accessed_element = emit_array_access(data.addr(), shape_, indices, env);

    return reference(accessed_element, data.origin());
}

expression reassociate_index_expression(expression expr)
{
    using pattern::_;

    util::index_t constant_term = 0;

    std::function<expression(expression)> collect_constant_terms = [&](expression expr)
    {
        pattern::variable<expression> lhs, rhs;
        pattern::variable<util::index_t> value;

        auto m = pattern::matcher<expression, expression>()
                     .case_(binary_operator(pattern::value(binary_op_tag::plus),
                                            integer_literal(value), rhs),
                            [&]
                            {
                                constant_term += value.get();

                                return collect_constant_terms(rhs.get());
                            })
                     .case_(binary_operator(pattern::value(binary_op_tag::plus), lhs,
                                            integer_literal(value)),
                            [&]
                            {
                                constant_term += value.get();

                                return collect_constant_terms(lhs.get());
                            })
                     .case_(binary_operator(pattern::value(binary_op_tag::plus), lhs, rhs),
                            [&]
                            {
                                return binary_operator_expr(binary_op_tag::plus,
                                                            collect_constant_terms(lhs.get()),
                                                            collect_constant_terms(rhs.get()));
                            })
                     .case_(_, [&](const expression& self)
                            {
                                return self;
                            });

        return pattern::match(expr, m);
    };

    auto simplified_expr = collect_constant_terms(expr);

    if (constant_term != 0)
    {
        return binary_operator_expr(binary_op_tag::plus, simplified_expr,
                                    integer_literal_expr(constant_term));
    }
    else
    {
        return simplified_expr;
    }
}

reference emit_tensor_access(const variable_declaration& tensor,
                             const std::vector<expression>& indices, llvm_environment& env,
                             compilation_context& ctx)
{
    expression linearized_index = integer_literal_expr(0);

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        auto extent =
            intrinsic_function_expr("extent", {variable_ref_expr(tensor), integer_literal_expr(i)});

        linearized_index = binary_operator_expr(
            binary_op_tag::plus,
            binary_operator_expr(binary_op_tag::multiplies, extent, linearized_index), indices[i]);
    }

    linearized_index = reassociate_index_expression(std::move(linearized_index));

    auto linearized_index_ = compile(linearized_index, env, ctx);

    auto tensor_ = ctx.symbol_table().at(tensor.id());

    auto& builder = env.builder();

    llvm::Value* data_ptr = builder.CreateConstInBoundsGEP2_32(tensor_.addr(), 0, 0, "data_ptr");

    auto data = load_from_ref(reference(data_ptr, tensor_.origin()), env, ctx);

    auto data_ref =
        reference(builder.CreateInBoundsGEP(data, load_from_ref(linearized_index_, env, ctx)),
                  tensor_.origin() / "data");

    if (auto code_region = ctx.current_code_region())
    {
        auto alias_info = code_region->register_access(tensor, indices, data_ref);

        data_ref.add_alias_info(alias_info);
    }

    return data_ref;
}

// TODO: Implement the index expression reassociation optimization for slices.
reference emit_array_slice_access(const reference& slice, const std::vector<llvm::Value*>& indices,
                                  llvm_environment& env, compilation_context& ctx)
{
    auto& builder = env.builder();

    llvm::Value* data_ptr = builder.CreateConstInBoundsGEP2_32(slice.addr(), 0, 0, "data_ptr");
    llvm::Value* shape_ptr = builder.CreateConstInBoundsGEP2_32(slice.addr(), 0, 1, "shape_ptr");
    llvm::Value* origin_ptr = builder.CreateConstInBoundsGEP2_32(slice.addr(), 0, 2, "origin_ptr");

    auto shape = load_from_ref(reference(shape_ptr, slice.origin()), env, ctx);
    auto data = load_from_ref(reference(data_ptr, slice.origin()), env, ctx);
    auto origin = load_from_ref(reference(origin_ptr, slice.origin()), env, ctx);

    std::vector<llvm::Value*> transformed_indices;
    transformed_indices.reserve(indices.size());

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        auto origin_component_ptr = builder.CreateConstInBoundsGEP1_32(origin, i);

        auto origin_component =
            load_from_ref(reference(origin_component_ptr, slice.origin() / "origin"), env, ctx);

        auto transformed_index = builder.CreateAdd(origin_component, indices[i], "", true, true);

        transformed_indices.push_back(transformed_index);
    }

    auto accessed_element = emit_array_access(reference(data, slice.origin() / "data"),
                                              reference(shape, slice.origin() / "shape"),
                                              transformed_indices, env, ctx);

    return accessed_element;
}

std::vector<llvm::Value*> permute_indices(const std::vector<llvm::Value*>& indices,
                                          const std::vector<util::index_t>& permutation)
{
    std::vector<llvm::Value*> permuted_indices;
    permuted_indices.reserve(indices.size());

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        permuted_indices.push_back(indices[permutation[i]]);
    }

    return permuted_indices;
}

reference compile(const expression& expr, llvm_environment& env, compilation_context& ctx)
{
    using pattern::_;

    auto& symbol_table = ctx.symbol_table();

    pattern::variable<binary_op_tag> btag;
    pattern::variable<unary_op_tag> utag;
    pattern::variable<expression> a, b, c, d;
    pattern::variable<boost::optional<expression>> opt_expr;
    pattern::variable<std::vector<expression>> expressions;

    pattern::variable<variable_declaration> idx;
    pattern::variable<variable_declaration> var;
    pattern::variable<function_declaration> plan;

    pattern::variable<util::index_t> ival;
    pattern::variable<float> fval;
    pattern::variable<double> dval;

    pattern::variable<std::string> name;

    pattern::variable<type> t;

    auto& builder = env.builder();

    auto m =
        pattern::make_matcher<expression, reference>()
            .case_(binary_operator(btag, a, b),
                   [&]
                   {
                       return emit_binary_operator(btag.get(), a.get(), b.get(), env, ctx);
                   })
            .case_(unary_operator(utag, a),
                   [&]
                   {
                       return emit_unary_operator(utag.get(), a.get(), env, ctx);
                   })
            .case_(for_(idx, a, b, c, d),
                   [&]
                   {
                       ctx.enter_new_scope();

                       llvm::Type* size_type = env.map_qubus_type(types::integer());

                       auto increment_ptr = compile(c.get(), env, ctx);
                       auto increment_value = load_from_ref(increment_ptr, env, ctx);

                       llvm::Value* induction_var = create_entry_block_alloca(
                           env.get_current_function(), size_type, nullptr, "ind");

                       auto induction_var_ref = reference(induction_var, access_path());

                       symbol_table[idx.get().id()] = induction_var_ref;

                       reference lower_bound_ptr = compile(a.get(), env, ctx);
                       reference upper_bound_ptr = compile(b.get(), env, ctx);

                       auto lower_bound = load_from_ref(lower_bound_ptr, env, ctx);
                       auto upper_bound = load_from_ref(upper_bound_ptr, env, ctx);

                       emit_loop(induction_var_ref, lower_bound, upper_bound, increment_value,
                                 [&]()
                                 {
                                     if (!contains_loops(d.get()))
                                     {
                                         ctx.enter_code_region(d.get());

                                         compile(d.get(), env, ctx);

                                         ctx.leave_code_region();
                                     }
                                     else
                                     {
                                         compile(d.get(), env, ctx);
                                     }
                                 },
                                 env, ctx);

                       return reference();
                   })
            .case_(if_(a, b, opt_expr),
                   [&]
                   {
                       auto condition = compile(a.get(), env, ctx);

                       emit_if_else(condition,
                                    [&]
                                    {
                                        compile(b.get(), env, ctx);
                                    },
                                    [&]
                                    {
                                        if (opt_expr.get())
                                        {
                                            compile(*opt_expr.get(), env, ctx);
                                        }
                                    },
                                    env, ctx);

                       return reference();
                   })
            .case_(compound(expressions),
                   [&]
                   {
                       for (const auto& subexpr : expressions.get())
                       {
                           compile(subexpr, env, ctx);
                       }

                       return reference();
                   })
            .case_(double_literal(dval),
                   [&]
                   {
                       llvm::Type* double_type = env.map_qubus_type(types::double_());

                       llvm::Value* value = llvm::ConstantFP::get(double_type, dval.get());

                       auto var =
                           create_entry_block_alloca(env.get_current_function(), double_type);

                       reference var_ref(var, access_path());
                       store_to_ref(var_ref, value, env, ctx);

                       return var_ref;
                   })
            .case_(float_literal(fval),
                   [&]
                   {
                       llvm::Type* float_type = env.map_qubus_type(types::float_());

                       llvm::Value* value = llvm::ConstantFP::get(float_type, fval.get());

                       llvm::Value* var =
                           create_entry_block_alloca(env.get_current_function(), float_type);

                       reference var_ref(var, access_path());
                       store_to_ref(var_ref, value, env, ctx);

                       return var_ref;
                   })
            .case_(integer_literal(ival),
                   [&]
                   {
                       llvm::Type* size_type = env.map_qubus_type(types::integer());

                       llvm::Value* value = llvm::ConstantInt::get(size_type, ival.get());

                       llvm::Value* var =
                           create_entry_block_alloca(env.get_current_function(), size_type);

                       reference var_ref(var, access_path());
                       store_to_ref(var_ref, value, env, ctx);

                       return var_ref;
                   })
            .case_(intrinsic_function_n(pattern::value("delta"), a, b),
                   [&]
                   {
                       // TODO: Test if a and b are integers.

                       auto int_type = env.map_qubus_type(types::integer());

                       auto a_ref = compile(a.get(), env, ctx);
                       auto b_ref = compile(b.get(), env, ctx);

                       auto a_value = load_from_ref(a_ref, env, ctx);

                       auto b_value = load_from_ref(b_ref, env, ctx);

                       auto cond = builder.CreateICmpEQ(a_value, b_value);

                       auto one = llvm::ConstantInt::get(int_type, 1);
                       auto zero = llvm::ConstantInt::get(int_type, 0);

                       auto result_value = builder.CreateSelect(cond, one, zero);

                       auto result =
                           create_entry_block_alloca(env.get_current_function(), int_type);

                       reference result_ref(result, access_path());
                       store_to_ref(result_ref, result_value, env, ctx);

                       return result_ref;
                   })
            .case_(intrinsic_function_n(pattern::value("extent"), variable_ref(idx), b),
                   [&]
                   {
                       auto tensor = symbol_table.at(idx.get().id());

                       llvm::Value* shape_ptr =
                           builder.CreateConstInBoundsGEP2_32(tensor.addr(), 0, 1, "shape_ptr");

                       auto shape = load_from_ref(reference(shape_ptr, tensor.origin()), env, ctx);

                       reference dim_ref = compile(b.get(), env, ctx);

                       auto dim = load_from_ref(dim_ref, env, ctx);

                       llvm::Value* extent_ptr =
                           builder.CreateInBoundsGEP(shape, dim, "extent_ptr");

                       return reference(extent_ptr, tensor.origin() / "shape");
                   })
            .case_(intrinsic_function_n(pattern::value("min"), a, b),
                   [&]
                   {
                       reference left_value_ptr = compile(a.get(), env, ctx);
                       reference right_value_ptr = compile(b.get(), env, ctx);

                       llvm::Instruction* left_value = load_from_ref(left_value_ptr, env, ctx);

                       llvm::Instruction* right_value = load_from_ref(right_value_ptr, env, ctx);

                       type result_type = typeof_(a.get());

                       auto cond = builder.CreateICmpSLT(left_value, right_value);

                       auto result = builder.CreateSelect(cond, left_value, right_value);

                       auto result_var =
                           create_entry_block_alloca(env.get_current_function(), result->getType());

                       reference result_ref(result_var, access_path());
                       store_to_ref(result_ref, result, env, ctx);

                       return result_ref;
                   })
            .case_(intrinsic_function_n(pattern::value("max"), a, b),
                   [&]
                   {
                       reference left_value_ptr = compile(a.get(), env, ctx);
                       reference right_value_ptr = compile(b.get(), env, ctx);

                       llvm::Instruction* left_value = load_from_ref(left_value_ptr, env, ctx);

                       llvm::Instruction* right_value = load_from_ref(right_value_ptr, env, ctx);

                       auto cond = builder.CreateICmpSLT(left_value, right_value);

                       auto result = builder.CreateSelect(cond, right_value, left_value);

                       auto result_var =
                           create_entry_block_alloca(env.get_current_function(), result->getType());

                       reference result_ref(result_var, access_path());
                       store_to_ref(result_ref, result, env, ctx);

                       return result_ref;
                   })
            .case_(intrinsic_function_n(pattern::value("select"), a, b, c),
                   [&]
                   {
                       reference cond_value_ptr = compile(a.get(), env, ctx);
                       reference then_value_ptr = compile(b.get(), env, ctx);
                       reference else_value_ptr = compile(c.get(), env, ctx);

                       llvm::Instruction* cond_value = load_from_ref(cond_value_ptr, env, ctx);
                       llvm::Instruction* then_value = load_from_ref(then_value_ptr, env, ctx);
                       llvm::Instruction* else_value = load_from_ref(else_value_ptr, env, ctx);

                       auto result = builder.CreateSelect(cond_value, then_value, else_value);

                       auto result_var =
                           create_entry_block_alloca(env.get_current_function(), result->getType());

                       reference result_ref(result_var, access_path());
                       store_to_ref(result_ref, result, env, ctx);

                       return result_ref;
                   })
            .case_(type_conversion(t, a),
                   [&]
                   {
                       return emit_type_conversion(t.get(), a.get(), env, ctx);
                   })
            .case_(variable_ref(idx),
                   [&]
                   {
                       return symbol_table.at(idx.get().id());
                   })
            .case_(subscription(variable_ref(idx), expressions),
                   [&]
                   {
                       auto ref = symbol_table.at(idx.get().id());

                       const auto& indices = expressions.get();

                       std::vector<llvm::Value*> indices_;
                       indices_.reserve(indices.size());

                       for (const auto& index : indices)
                       {
                           auto index_ref = compile(index, env, ctx);

                           auto index_ = load_from_ref(index_ref, env, ctx);

                           indices_.push_back(index_);
                       }

                       using pattern::_;

                       auto m =
                           pattern::make_matcher<type, reference>()
                               .case_(pattern::tensor_t(_),
                                      [&]
                                      {
                                          return emit_tensor_access(idx.get(), indices, env, ctx);
                                      })
                               .case_(pattern::array_slice_t(_), [&]
                                      {
                                          return emit_array_slice_access(ref, indices_, env, ctx);
                                      });

                       return pattern::match(idx.get().var_type(), m);
                   })
            .case_(local_variable_def(var, a),
                   [&]
                   {
                       auto var_type = env.map_qubus_type(var.get().var_type());

                       auto var_ptr =
                           create_entry_block_alloca(env.get_current_function(), var_type);

                       auto init_value_ref = compile(a.get(), env, ctx);

                       auto init_value = load_from_ref(init_value_ref, env, ctx);

                       reference var_ref(var_ptr, access_path(var.get().id()));
                       store_to_ref(var_ref, init_value, env, ctx);

                       symbol_table[var.get().id()] = var_ref;

                       return reference();
                   })
            .case_(spawn(plan, expressions),
                   [&]
                   {
                       std::vector<llvm::Type*> param_types;

                       for (const auto& param : plan.get().params())
                       {
                           param_types.push_back(
                               env.map_qubus_type(param.var_type())->getPointerTo());
                       }

                       param_types.push_back(
                           env.map_qubus_type(plan.get().result().var_type())->getPointerTo());

                       param_types.push_back(llvm::PointerType::get(
                           llvm::Type::getInt8Ty(llvm::getGlobalContext()), 0));

                       llvm::FunctionType* fn_type = llvm::FunctionType::get(
                           llvm::Type::getVoidTy(llvm::getGlobalContext()), param_types, false);

                       auto plan_ptr =
                           llvm::Function::Create(fn_type, llvm::Function::PrivateLinkage,
                                                  plan.get().name(), &env.module());

                       for (std::size_t i = 0; i < plan_ptr->arg_size(); ++i)
                       {
                           plan_ptr->setDoesNotAlias(i + 1);
                       }

                       ctx.add_plan_to_compile(plan.get());

                       std::vector<llvm::Value*> arguments;

                       for (const auto& arg : expressions.get())
                       {
                           auto arg_ref = compile(arg, env, ctx);

                           arguments.push_back(arg_ref.addr());
                       }

                       arguments.push_back(&env.get_current_function()->getArgumentList().back());

                       builder.CreateCall(plan_ptr, arguments);

                       return reference();
                   })
            .case_(
                construct(t, expressions), [&]
                {
                    pattern::variable<type> value_type;

                    auto m =
                        pattern::make_matcher<type, llvm::Value*>()
                            .case_(
                                 tensor_t(value_type) || array_t(value_type),
                                 [&](const type& self)
                                 {
                                     auto size_type = env.map_qubus_type(types::integer());

                                     const auto& args = expressions.get();

                                     std::vector<util::index_t> extents;

                                     try
                                     {
                                         for (const auto& arg : args)
                                         {
                                             extents.push_back(
                                                 arg.as<integer_literal_expr>().value());
                                         }
                                     }
                                     catch (const std::bad_cast&)
                                     {
                                         throw 0;
                                     }

                                     std::size_t mem_size = 8;

                                     for (auto extent : extents)
                                     {
                                         mem_size *= extent;
                                     }

                                     llvm::Value* data_ptr;

                                     if (mem_size < 256)
                                     {
                                         llvm::Type* multi_array_type =
                                             env.map_qubus_type(value_type.get());

                                         std::size_t size = 1;

                                         for (auto extent : extents)
                                         {
                                             size *= extent;
                                         }

                                         multi_array_type =
                                             llvm::ArrayType::get(multi_array_type, size);

                                         data_ptr = builder.CreateConstInBoundsGEP2_64(
                                             create_entry_block_alloca(env.get_current_function(),
                                                                       multi_array_type),
                                             0, 0);
                                     }
                                     else
                                     {
                                         std::vector<llvm::Value*> args;

                                         args.push_back(
                                             &env.get_current_function()->getArgumentList().back());
                                         args.push_back(
                                             llvm::ConstantInt::get(size_type, mem_size));

                                         data_ptr = builder.CreateBitCast(
                                             builder.CreateCall(env.get_alloc_scratch_mem(), args),
                                             env.map_qubus_type(value_type.get())->getPointerTo(0));

                                         ctx.get_current_scope().on_exit(
                                             [args, &env, &builder]
                                             {
                                                 builder.CreateCall(env.get_dealloc_scratch_mem(),
                                                                    args);
                                             });
                                     }

                                     auto shape_ptr = builder.CreateConstInBoundsGEP2_64(
                                         create_entry_block_alloca(
                                             env.get_current_function(),
                                             llvm::ArrayType::get(size_type, extents.size())),
                                         0, 0, "shape_ptr");

                                     for (std::size_t i = 0; i < extents.size(); ++i)
                                     {
                                         auto extent_ptr = builder.CreateConstInBoundsGEP1_64(
                                             shape_ptr, i, "extend_ptr");

                                         store_to_ref(reference(extent_ptr, access_path()),
                                                      llvm::ConstantInt::get(size_type, extents[i]),
                                                      env, ctx);
                                     }

                                     auto array_ptr = create_entry_block_alloca(
                                         env.get_current_function(), env.map_qubus_type(self));

                                     auto data_member_ptr =
                                         builder.CreateConstInBoundsGEP2_32(array_ptr, 0, 0);
                                     store_to_ref(reference(data_member_ptr, access_path()),
                                                  data_ptr, env, ctx);

                                     auto shape_member_ptr =
                                         builder.CreateConstInBoundsGEP2_32(array_ptr, 0, 1);
                                     store_to_ref(reference(shape_member_ptr, access_path()),
                                                  shape_ptr, env, ctx);

                                     return array_ptr;
                                 })
                            .case_(value_type, [&]
                                   {
                                       if (expressions.get().size() != 0)
                                           throw 0;

                                       return create_entry_block_alloca(
                                           env.get_current_function(),
                                           env.map_qubus_type(value_type.get()));
                                   });

                    return reference(pattern::match(t.get(), m), access_path());
                });

    auto result = pattern::match(expr, m);

    auto m2 = pattern::make_matcher<expression, void>().case_(variable_scope(_, _), [&]
                                                              {
                                                                  ctx.exit_current_scope();
                                                              });

    pattern::try_match(expr, m2);

    return result;
}

void compile(const function_declaration& plan, llvm_environment& env, compilation_context& ctx)
{
    ctx.enter_new_scope();

    auto& symbol_table = ctx.symbol_table();

    // Prolog
    std::vector<llvm::Type*> param_types;

    for (const auto& param : plan.params())
    {
        param_types.push_back(env.map_qubus_type(param.var_type())->getPointerTo());
    }

    param_types.push_back(env.map_qubus_type(plan.result().var_type())->getPointerTo());

    param_types.push_back(
        llvm::PointerType::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), 0));

    llvm::FunctionType* FT = llvm::FunctionType::get(
        llvm::Type::getVoidTy(llvm::getGlobalContext()), param_types, false);

    llvm::Function* compiled_plan;

    if (!(compiled_plan = env.module().getFunction(plan.name())))
    {
        compiled_plan =
            llvm::Function::Create(FT, llvm::Function::PrivateLinkage, plan.name(), &env.module());
    }

    for (std::size_t i = 0; i < compiled_plan->arg_size(); ++i)
    {
        compiled_plan->setDoesNotAlias(i + 1);
    }

    env.set_current_function(compiled_plan);

    llvm::BasicBlock* BB =
        llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", compiled_plan);
    env.builder().SetInsertPoint(BB);

    // body

    auto current_arg = compiled_plan->arg_begin();

    for (const auto& param : plan.params())
    {
        access_path apath(param.id());
        symbol_table[param.id()] = reference(&*current_arg, apath);
        env.get_alias_scope(apath);
        ++current_arg;
    }

    symbol_table[plan.result().id()] = reference(&*current_arg, access_path(plan.result().id()));

    compile(plan.body(), env, ctx);

    ctx.exit_current_scope();

    // Epilog
    env.builder().CreateRetVoid();
}

void compile_entry_point(const function_declaration& plan, llvm_environment& env, std::size_t id)
{
    compilation_context ctx(env);

    compile(plan, env, ctx);

    // Prolog
    std::vector<llvm::Type*> param_types;

    param_types.push_back(llvm::PointerType::get(
        llvm::PointerType::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), 0), 0));
    param_types.push_back(
        llvm::PointerType::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), 0));

    llvm::FunctionType* FT = llvm::FunctionType::get(
        llvm::Type::getVoidTy(llvm::getGlobalContext()), param_types, false);

    std::string entry_point_name = "qubus_cpu_plan" + std::to_string(id);

    llvm::Function* kernel = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                                    entry_point_name, &env.module());

    for (std::size_t i = 0; i < kernel->arg_size(); ++i)
    {
        kernel->setDoesNotAlias(i + 1);
    }

    env.set_current_function(kernel);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", kernel);
    env.builder().SetInsertPoint(BB);

    // unpack args
    std::size_t counter = 0;

    std::vector<llvm::Value*> arguments;

    auto add_param = [&](const variable_declaration& param) mutable
    {
        llvm::Type* param_type = env.map_qubus_type(param.var_type());

        llvm::Value* ptr_to_arg =
            env.builder().CreateConstInBoundsGEP1_64(&kernel->getArgumentList().front(), counter);

        auto arg = load_from_ref(reference(ptr_to_arg, access_path()), env, ctx);

        llvm::Value* typed_arg =
            env.builder().CreateBitCast(arg, llvm::PointerType::get(param_type, 0));

        llvm::Value* data_ptr =
            env.builder().CreateConstInBoundsGEP2_32(typed_arg, 0, 0, "data_ptr");

        auto data = load_from_ref(reference(data_ptr, access_path(param.id())), env, ctx);

        // TODO: get alignement from the ABI
        env.builder().CreateCall2(
            env.get_assume_align(), data,
            llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), 32));

        arguments.push_back(typed_arg);
        ++counter;
    };

    for (const auto& param : plan.params())
    {
        add_param(param);
    }

    add_param(plan.result());

    arguments.push_back(&kernel->getArgumentList().back());

    // body

    env.builder().CreateCall(env.module().getFunction(plan.name()), arguments);

    ctx.exit_current_scope();

    // Epilog
    env.builder().CreateRetVoid();

    // Emit all other plans.
    while (auto plan = ctx.get_next_plan_to_compile())
    {
        compile(*plan, env, ctx);
    }
}

class cpu_plan
{
public:
    explicit cpu_plan(void* entry_) : entry_(entry_)
    {
    }

    cpu_plan(const cpu_plan&) = delete;
    cpu_plan& operator=(const cpu_plan&) = delete;

    void execute(const std::vector<void*>& args, cpu_runtime& runtime) const
    {
        using entry_t = void (*)(void* const*, void*);

        auto entry = reinterpret_cast<entry_t>(entry_);

        entry(args.data(), &runtime);
    }

private:
    void* entry_;
};

void setup_function_optimization_pipeline(llvm::FunctionPassManager& manager, bool optimize)
{
    using namespace llvm;

    if (!optimize)
        return;

    manager.add(createBasicAliasAnalysisPass());
    manager.add(createScalarEvolutionAliasAnalysisPass());
    manager.add(createTypeBasedAliasAnalysisPass());
    manager.add(createScopedNoAliasAAPass());

    manager.add(createCFGSimplificationPass());
    manager.add(createSROAPass());
    manager.add(createEarlyCSEPass());
    manager.add(createLowerExpectIntrinsicPass());
}

void setup_optimization_pipeline(llvm::PassManager& manager, bool optimize, bool vectorize)
{
    using namespace llvm;

    if (!optimize)
        return;

    manager.add(createBasicAliasAnalysisPass());
    manager.add(createScalarEvolutionAliasAnalysisPass());
    manager.add(createTypeBasedAliasAnalysisPass());
    manager.add(createScopedNoAliasAAPass());

    manager.add(createIPSCCPPass());          // IP SCCP
    manager.add(createGlobalOptimizerPass()); // Optimize out global vars

    manager.add(createDeadArgEliminationPass()); // Dead argument elimination

    manager.add(createInstructionCombiningPass()); // Clean up after IPCP & DAE

    manager.add(createCFGSimplificationPass()); // Clean up after IPCP & DAE

    manager.add(createFunctionInliningPass());

    manager.add(createFunctionAttrsPass()); // Set readonly/readnone attrs

    manager.add(createArgumentPromotionPass()); // Scalarize uninlined fn args

    manager.add(createSROAPass(false));

    manager.add(createEarlyCSEPass());                   // Catch trivial redundancies
    manager.add(createJumpThreadingPass());              // Thread jumps.
    manager.add(createCorrelatedValuePropagationPass()); // Propagate conditionals
    manager.add(createCFGSimplificationPass());          // Merge & remove BBs
    manager.add(createInstructionCombiningPass());       // Combine silly seq's

    manager.add(createTailCallEliminationPass()); // Eliminate tail calls

    manager.add(createCFGSimplificationPass()); // Merge & remove BBs
    manager.add(createReassociatePass());       // Reassociate expressions
    // Rotate Loop - disable header duplication at -Oz
    manager.add(createLoopRotatePass(-1));
    manager.add(createLICMPass()); // Hoist loop invariants
    manager.add(createLoopUnswitchPass(false));
    manager.add(createInstructionCombiningPass());
    manager.add(createIndVarSimplifyPass()); // Canonicalize indvars
    // manager.add(createLoopIdiomPass());             // Recognize idioms like memset.
    manager.add(createLoopDeletionPass()); // Delete dead loops

    manager.add(createMergedLoadStoreMotionPass());
    manager.add(createGVNPass(false));

    // manager.add(createMemCpyOptPass());
    manager.add(createSCCPPass()); // Constant prop with SCCP

    manager.add(createInstructionCombiningPass());

    manager.add(createJumpThreadingPass()); // Thread jumps
    manager.add(createCorrelatedValuePropagationPass());
    manager.add(createDeadStoreEliminationPass()); // Delete dead stores
    manager.add(createLICMPass());

    manager.add(createLoadCombinePass());

    manager.add(createAggressiveDCEPass());        // Delete dead instructions
    manager.add(createCFGSimplificationPass());    // Merge & remove BBs
    manager.add(createInstructionCombiningPass()); // Clean up after everything.

    manager.add(createBarrierNoopPass());

    // vectorization

    if (vectorize)
    {
        manager.add(createLoopRotatePass());

        manager.add(createLoopVectorizePass(true, true));

        manager.add(createInstructionCombiningPass());

        manager.add(createEarlyCSEPass());
        manager.add(createCorrelatedValuePropagationPass());
        manager.add(createInstructionCombiningPass());
        manager.add(createLICMPass());
        manager.add(createLoopUnswitchPass(false));
        manager.add(createCFGSimplificationPass());
        manager.add(createInstructionCombiningPass());

        manager.add(createSLPVectorizerPass()); // Vectorize parallel scalar chains.
        manager.add(createEarlyCSEPass());
    }

    // end vectorization

    manager.add(createCFGSimplificationPass());
    manager.add(createInstructionCombiningPass());

    manager.add(createAlignmentFromAssumptionsPass());

    manager.add(createStripDeadPrototypesPass()); // Get rid of dead prototypes

    manager.add(createGlobalDCEPass());     // Remove dead fns and globals.
    manager.add(createConstantMergePass()); // Merge dup global constants

    manager.add(createMergeFunctionsPass());
}

std::unique_ptr<cpu_plan> compile(function_declaration entry_point, llvm::ExecutionEngine& engine)
{
    static std::size_t unique_id = 0;

    llvm_environment env;

    compile_entry_point(entry_point, env, unique_id);

    std::unique_ptr<llvm::Module> the_module = env.detach_module();

    the_module->setDataLayout(engine.getDataLayout());

    llvm::verifyModule(*the_module);

    // the_module->dump();
    // std::cout << std::endl;

    llvm::FunctionPassManager fn_pass_man(the_module.get());
    llvm::PassManager pass_man;

    fn_pass_man.add(new llvm::DataLayoutPass());
    pass_man.add(new llvm::DataLayoutPass());

    engine.getTargetMachine()->addAnalysisPasses(fn_pass_man);
    engine.getTargetMachine()->addAnalysisPasses(pass_man);

    setup_function_optimization_pipeline(fn_pass_man, true);
    setup_optimization_pipeline(pass_man, true, true);

    fn_pass_man.doInitialization();

    for (auto& fn : *the_module)
    {
        fn_pass_man.run(fn);
    }

    fn_pass_man.doFinalization();

    pass_man.run(*the_module);

    // the_module->dump();
    // std::cout << std::endl;

    /*std::cout << "The assembler output:\n\n";

    llvm::raw_os_ostream m3log(std::cout);
    llvm::formatted_raw_ostream fm3log(m3log);

    llvm::PassManager pMPasses;
    // pMPasses.add(new llvm::DataLayoutPass(*engine.getDataLayout()));
    engine.getTargetMachine()->addPassesToEmitFile(pMPasses, fm3log,
                                                   llvm::TargetMachine::CGFT_AssemblyFile);
    pMPasses.run(*the_module);*/

    engine.finalizeObject();

    engine.addModule(std::move(the_module));

    return util::make_unique<cpu_plan>(reinterpret_cast<void*>(
        engine.getFunctionAddress("qubus_cpu_plan" + std::to_string(unique_id++))));
}

class cpu_plan_registry
{
public:
    plan register_plan(std::unique_ptr<cpu_plan> p, std::vector<intent> intents)
    {
        auto plan_handle = handle_fac_.create();

        plans_.emplace(plan_handle, std::move(p));

        return plan(plan_handle, std::move(intents));
    }

    const cpu_plan& lookup_plan(const plan& handle) const
    {
        return *plans_.at(handle.id());
    }

private:
    util::handle_factory handle_fac_;
    std::unordered_map<util::handle, std::unique_ptr<cpu_plan>> plans_;
};
}

class cpu_compiler : public compiler
{
public:
    explicit cpu_compiler(cpu_plan_registry& plan_registry_) : plan_registry_(&plan_registry_)
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();

        llvm::EngineBuilder builder(
            util::make_unique<llvm::Module>("dummy", llvm::getGlobalContext()));

        std::vector<std::string> available_features;
        llvm::StringMap<bool> features;

        if (llvm::sys::getHostCPUFeatures(features))
        {
            for (const auto& feature : features)
            {
                std::cout << std::string(feature.getKey()) << std::endl;
                if (feature.getValue())
                {
                    available_features.push_back(feature.getKey());
                    std::cout << std::string(feature.getKey()) << std::endl;
                }
            }
        }
        else
        {
            builder.setMCPU(llvm::sys::getHostCPUName());

            available_features = deduce_host_cpu_features();
        }

        builder.setMAttrs(available_features);
        builder.setOptLevel(llvm::CodeGenOpt::Aggressive);

        llvm::TargetOptions options;
        options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
        options.UnsafeFPMath = 1;
        options.NoInfsFPMath = 1;
        options.NoNaNsFPMath = 1;

        builder.setTargetOptions(options);

        engine_ = std::unique_ptr<llvm::ExecutionEngine>(builder.create());

#if LLVM_USE_INTEL_JITEVENTS
        llvm::JITEventListener* vtuneProfiler =
            llvm::JITEventListener::createIntelJITEventListener();
        engine_->RegisterJITEventListener(vtuneProfiler);
#endif
    }

    virtual ~cpu_compiler() = default;

    plan compile_plan(const function_declaration& plan_decl) override
    {
        auto param_count = plan_decl.params().size();

        std::vector<intent> intents(param_count, intent::in);
        intents.push_back(intent::inout);

        auto compiled_plan = compile(plan_decl, *engine_);

        return plan_registry_->register_plan(std::move(compiled_plan), std::move(intents));
    }

private:
    std::unique_ptr<llvm::ExecutionEngine> engine_;
    cpu_plan_registry* plan_registry_;
};

class cpu_executor : public executor
{
public:
    cpu_executor(cpu_plan_registry& plan_registry_, local_address_space& addr_space_,
                 const abi_info& abi_)
    : plan_registry_(&plan_registry_), addr_space_(&addr_space_), abi_(&abi_), exec_stack_(4 * 1024)
    {
    }

    virtual ~cpu_executor() = default;

    hpx::lcos::future<void> execute_plan(const plan& executed_plan, execution_context ctx) override
    {
        const cpu_plan& executed_cpu_plan = plan_registry_->lookup_plan(executed_plan);

        std::vector<void*> plan_args;

        std::vector<std::shared_ptr<memory_block>> used_mem_blocks;

        for (const auto& arg : ctx.args())
        {
            plan_args.push_back(
                build_object_metadata(*arg, *addr_space_, *abi_, exec_stack_, used_mem_blocks));
        }

        return hpx::async([&executed_cpu_plan, plan_args, used_mem_blocks, this]
                          {
                              cpu_runtime runtime;

                              executed_cpu_plan.execute(plan_args, runtime);
                              exec_stack_.clear();
                          });
    }

private:
    cpu_plan_registry* plan_registry_;
    local_address_space* addr_space_;
    const abi_info* abi_;
    execution_stack exec_stack_;
};

class cpu_backend final : public backend
{
public:
    cpu_backend(const abi_info& abi_)
    : addr_space_(util::make_unique<local_address_space>(util::make_unique<cpu_allocator>())),
      obj_factory_(util::make_unique<cpu_object_factory>(addr_space_->get_allocator(), abi_)),
      compiler_(util::make_unique<cpu_compiler>(plan_registry_)),
      executor_(util::make_unique<cpu_executor>(plan_registry_, *addr_space_, abi_))
    {
        // use hwloc to obtain informations over all local CPUs
    }

    virtual ~cpu_backend() = default;

    std::string id() const override
    {
        return "qubus.cpu";
    }

    std::vector<executor*> executors() const override
    {
        return {executor_.get()};
    }

    compiler& get_compiler() const override
    {
        return *compiler_;
    }

    local_object_factory& local_factory() const override
    {
        return *obj_factory_;
    }

    local_address_space& address_space() const override
    {
        return *addr_space_;
    }

private:
    std::unique_ptr<local_address_space> addr_space_;
    std::unique_ptr<cpu_object_factory> obj_factory_;
    cpu_plan_registry plan_registry_;
    std::unique_ptr<cpu_compiler> compiler_;
    std::unique_ptr<cpu_executor> executor_;
};

std::unique_ptr<cpu_backend> the_cpu_backend;
std::once_flag cpu_backend_init_flag;

extern "C" QBB_QUBUS_EXPORT backend* init_cpu_backend(const abi_info* abi)
{
    std::call_once(cpu_backend_init_flag, [&]
                   {
                       the_cpu_backend = std::make_unique<cpu_backend>(*abi);
                   });

    return the_cpu_backend.get();
}
}
}
