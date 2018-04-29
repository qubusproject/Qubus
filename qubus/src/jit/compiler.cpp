#include <qubus/jit/compiler.hpp>

#include <qubus/jit/compile.hpp>

#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <boost/optional.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/make_unique.hpp>
#include <qubus/util/unused.hpp>

#include <iterator>
#include <memory>
#include <utility>
#include <vector>

namespace qubus
{
namespace jit
{

namespace
{

void generate_foreign_function_interface(const function& plan, compiler& comp)
{
    auto& env = comp.get_module().env();
    auto& ctx = comp.get_module().ctx();

    ctx.enter_new_scope();

    // Prolog
    std::vector<llvm::Type*> param_types;

    auto generic_ptr_type = llvm::PointerType::get(llvm::Type::getInt8Ty(env.ctx()), 0);

    param_types.push_back(generic_ptr_type->getPointerTo(0));
    param_types.push_back(generic_ptr_type);

    llvm::FunctionType* FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(env.ctx()), param_types, false);

    auto mangled_foreign_name = comp.mangle_foreign_function_name(symbol_id(plan.full_name()));

    llvm::Function* kernel = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                                    mangled_foreign_name, &env.module());

    for (std::size_t i = 0; i < kernel->arg_size(); ++i)
    {
        kernel->addAttribute(i + 1, llvm::Attribute::AttrKind::NoAlias);
    }

    env.set_current_function(kernel);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(env.ctx(), "entry", kernel);
    env.builder().SetInsertPoint(BB);

    // unpack args
    std::size_t counter = 0;

    std::vector<llvm::Value*> parameters;

    auto add_param = [&](const variable_declaration& param) mutable {
        llvm::Type* param_type = env.map_qubus_type(param.var_type());

        llvm::Value* ptr_to_arg =
            env.builder().CreateConstInBoundsGEP1_64(&*kernel->arg_begin(), counter);

        auto arg = env.builder().CreateLoad(ptr_to_arg);

        llvm::Value* typed_arg =
            env.builder().CreateBitCast(arg, llvm::PointerType::get(param_type, 0));

        // TODO: Generalize and reenable alignment tracking.
        // TODO: get alignment from the ABI

        parameters.push_back(typed_arg);

        ++counter;
    };

    for (const auto& param : plan.params())
    {
        add_param(param);
    }

    add_param(plan.result());

    parameters.push_back(&*std::next(kernel->arg_begin()));

    // body

    auto mangled_name = comp.mangle_function_name(symbol_id(plan.full_name()));

    llvm::Function* func = env.module().getFunction(mangled_name);

    QUBUS_ASSERT(func, "Function not found.");

    env.builder().CreateCall(func, parameters);

    ctx.exit_current_scope();

    // Epilog
    env.builder().CreateRetVoid();
}

void compile_function(const function& plan, compiler& comp)
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

    param_types.push_back(llvm::PointerType::get(llvm::Type::getInt8Ty(env.ctx()), 0));

    llvm::FunctionType* FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(env.ctx()), param_types, false);

    auto mangled_name = comp.mangle_function_name(symbol_id(plan.full_name()));

    llvm::Function* compiled_plan;

    if ((compiled_plan = env.module().getFunction(mangled_name)) == nullptr)
    {
        compiled_plan =
            llvm::Function::Create(FT, llvm::Function::PrivateLinkage, mangled_name, &env.module());
    }

    for (std::size_t i = 0; i < compiled_plan->arg_size(); ++i)
    {
        compiled_plan->addAttribute(i + 1, llvm::Attribute::AttrKind::NoAlias);
    }

    env.set_current_function(compiled_plan);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(env.ctx(), "entry", compiled_plan);
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

    generate_foreign_function_interface(plan, comp);
}

} // namespace

module::module(llvm::LLVMContext& llvm_ctx_)
: env_(llvm_ctx_), ctx_(std::make_unique<compilation_context>(env_))
{
}

std::unique_ptr<llvm::Module> module::detach_module()
{
    return env().detach_module();
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

compiler::compiler() : ctx_(std::make_unique<llvm::LLVMContext>()), current_module_(nullptr)
{
}

std::string compiler::mangle_function_name(const qubus::symbol_id& func_name)
{
    std::string result;

    for (auto iter = func_name.components().begin(), end = func_name.components().end();
         iter != end; ++iter)
    {
        result += *iter;

        if (iter != end - 1)
        {
            result += '_';
        }
    }

    return result;
}

std::string compiler::mangle_foreign_function_name(const qubus::symbol_id& func_name)
{
    return mangle_function_name(func_name) + "_ffi";
}

reference compiler::compile(const expression& expr)
{
    return ::qubus::jit::compile(expr, *this);
}

void compiler::compile(const function& func)
{
    return compile_function(func, *this);
}

reference compiler::compile_root_skeleton(const expression& root_skel)
{
    return compile(root_skel);
}

void compiler::set_module(module& current_module)
{
    current_module_ = &current_module;
}

module& compiler::get_module()
{
    return *current_module_;
}

llvm::LLVMContext& compiler::get_context()
{
    return *ctx_;
}

void compiler::reset()
{
    current_module_->finish();

    current_module_ = nullptr;
}

std::unique_ptr<module> compile(std::unique_ptr<::qubus::module> mod, compiler& comp)
{
    auto compiled_module = std::make_unique<module>(comp.get_context());

    comp.set_module(*compiled_module);

    for (const auto& func : mod->functions())
    {
        comp.compile(func);
    }

    comp.reset();

    return compiled_module;
}
} // namespace jit
} // namespace qubus
