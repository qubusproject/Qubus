#include <qbb/qubus/jit/compiler.hpp>

#include <qbb/qubus/jit/compile.hpp>

#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <boost/optional.hpp>

#include <qbb/util/make_unique.hpp>
#include <qbb/util/assert.hpp>
#include <qbb/util/unused.hpp>

#include <memory>
#include <vector>
#include <utility>

namespace qbb
{
namespace qubus
{
namespace jit
{

namespace
{

void compile_function(const function_declaration& plan, compiler& comp)
{
    auto& env = comp.get_module().env();
    auto& ctx = comp.get_module().ctx();

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
        symbol_table[param.id()] = reference(&*current_arg, apath, param.var_type());
        env.get_alias_scope(apath);
        ++current_arg;
    }

    symbol_table[plan.result().id()] =
        reference(&*current_arg, access_path(plan.result().id()), plan.result().var_type());

    comp.compile(plan.body());

    ctx.exit_current_scope();

    // Epilog
    env.builder().CreateRetVoid();
}

void compile_entry_point(const function_declaration& plan, compiler& comp, const std::string& namespace_)
{
    auto& env = comp.get_module().env();
    auto& ctx = comp.get_module().ctx();

    comp.compile(plan);

    // Prolog
    std::vector<llvm::Type*> param_types;

    param_types.push_back(llvm::PointerType::get(
        llvm::PointerType::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), 0), 0));
    param_types.push_back(
        llvm::PointerType::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), 0));

    llvm::FunctionType* FT = llvm::FunctionType::get(
        llvm::Type::getVoidTy(llvm::getGlobalContext()), param_types, false);

    std::string entry_point_name = namespace_;

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

        auto arg = env.builder().CreateLoad(env.builder().getInt8PtrTy(0), ptr_to_arg);

        llvm::Value* typed_arg =
            env.builder().CreateBitCast(arg, llvm::PointerType::get(param_type, 0));

        // TODO: Generalize and reenable alignment tracking.
        /*llvm::Value* data_ptr =
            env.builder().CreateConstInBoundsGEP2_32(typed_arg, 0, 0, "data_ptr");

        auto data = load_from_ref(reference(data_ptr, access_path(param.id())), env, ctx);

        // TODO: get alignement from the ABI
        env.builder().CreateCall2(
            env.get_assume_align(),
            env.builder().CreateBitCast(data, llvm::Type::getInt8PtrTy(llvm::getGlobalContext())),
            llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), 32));*/

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
        comp.compile(*plan);
    }
}
}

module::module(std::string namespace_) : namespace_(std::move(namespace_)), ctx_(std::make_unique<compilation_context>(env_))
{
}

const std::string& module::get_namespace() const
{
    return namespace_;
}

llvm_environment& module::env()
{
    return env_;
}

compilation_context& module::ctx()
{
    return *ctx_;
}

void module::finish()
{
    ctx_.reset();
}

reference compiler::compile(const expression& expr)
{
    return ::qbb::qubus::jit::compile(expr, *this);
}

void compiler::compile(const function_declaration& func)
{
    return compile_function(func, *this);
}

void compiler::compile_entry_function(const function_declaration& func)
{

    return compile_entry_point(func, *this, current_module_->get_namespace());
}

void compiler::set_module(module& current_module)
{
    current_module_ = &current_module;
}

module& compiler::get_module()
{
    return *current_module_;
}

void compiler::reset()
{
    current_module_->finish();

    current_module_ = nullptr;
}

std::unique_ptr<module> compile(const function_declaration& func, compiler& comp)
{
    static std::size_t unique_id = 0;

    auto mod = std::make_unique<module>("qubus_plan" + std::to_string(unique_id));

    ++unique_id;

    comp.set_module(*mod);

    comp.compile_entry_function(func);

    comp.reset();

    return mod;
}
}
}
}
