#include <qubus/jit/compiler.hpp>

#include <qubus/jit/compile.hpp>
#include <qubus/jit/control_flow.hpp>
#include <qubus/jit/entry_block_alloca.hpp>
#include <qubus/jit/load_store.hpp>

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

module::module(std::string_view name_, std::shared_ptr<llvm::LLVMContext> ctx_)
: ctx_(std::move(ctx_)), llvm_module_(std::make_unique<llvm::Module>(llvm::StringRef(name_.data(), name_.length()), *this->ctx_))
{
}

qubus::jit::function& module::create_function(const qubus::function& func, compiler& comp)
{
    auto& env = comp.env();
    auto& ctx = comp.compiler_ctx();
    auto& symbol_table = ctx.symbol_table();

    std::vector<llvm::Type*> param_types;

    for (const auto& param : func.params())
    {
        param_types.push_back(env.map_qubus_type(param.var_type())->getPointerTo());
    }

    param_types.push_back(env.map_qubus_type(func.result().var_type())->getPointerTo());

    param_types.push_back(llvm::PointerType::get(llvm::Type::getInt8Ty(env.ctx()), 0));

    llvm::FunctionType* FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(env.ctx()), param_types, false);

    auto mangled_name = comp.mangle_function_name(symbol_id(func.full_name()));

    auto function_impl =
        llvm::Function::Create(FT, llvm::Function::PrivateLinkage, mangled_name, llvm_module_.get());

    for (std::size_t i = 0; i < function_impl->arg_size(); ++i)
    {
        function_impl->addAttribute(i + 1, llvm::Attribute::AttrKind::NoAlias);
    }

    env.set_current_function(function_impl);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(env.ctx(), "entry", function_impl);
    env.builder().SetInsertPoint(BB);

    // body

    auto current_arg = function_impl->arg_begin();

    for (const auto& param : func.params())
    {
        access_path apath(param.id());
        symbol_table[param.id()] = reference(&*current_arg, apath, param.var_type());
        env.get_alias_scope(apath);
        ++current_arg;
    }

    symbol_table[func.result().id()] =
        reference(&*current_arg, access_path(func.result().id()), func.result().var_type());

    comp.compile(func.body());

    ctx.exit_current_scope();

    // Epilog
    env.builder().CreateRetVoid();

    return functions_.emplace_back(func, *function_impl);
}

qubus::jit::exported_function& module::created_exported_function(const qubus::function& func, compiler& comp)
{
    qubus::jit::function& jitted_func = create_function(func, comp);

    return created_exported_function(jitted_func, comp);
}

qubus::jit::exported_function& module::created_exported_function(const qubus::jit::function& func, compiler& comp)
{
    auto& env = comp.env();
    auto& ctx = comp.compiler_ctx();

    ctx.enter_new_scope();

    // Prolog
    std::vector<llvm::Type*> param_types;

    auto generic_ptr_type = llvm::PointerType::get(llvm::Type::getInt8Ty(env.ctx()), 0);

    param_types.push_back(generic_ptr_type->getPointerTo(0));
    param_types.push_back(generic_ptr_type);

    llvm::FunctionType* FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(env.ctx()), param_types, false);

    auto mangled_foreign_name = comp.mangle_foreign_function_name(symbol_id(func.definition().full_name()));

    llvm::Function* kernel = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                                    mangled_foreign_name, llvm_module_.get());

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

    for (const auto& param : func.definition().params())
    {
        add_param(param);
    }

    add_param(func.definition().result());

    parameters.push_back(&*std::next(kernel->arg_begin()));

    // body

    env.builder().CreateCall(&func.code(), parameters);

    ctx.exit_current_scope();

    // Epilog
    env.builder().CreateRetVoid();

    return exported_functions_.emplace_back(func, *kernel);
}

qubus::jit::user_defined_type& module::create_user_defined_type(types::struct_ type_definition, compiler &comp)
{
    auto generic_ptr_type = llvm::PointerType::get(llvm::Type::getInt8Ty(comp.get_context()), 0);

    auto member_table_type =
            llvm::ArrayType::get(generic_ptr_type, type_definition.member_count());

    llvm::StructType* realisation = llvm::StructType::create({member_table_type}, type_definition.id());

    return user_defined_types_.emplace_back(std::move(type_definition), *realisation);
}

llvm::Module& module::llvm_module()
{
    return *llvm_module_;
}

const llvm::Module& module::llvm_module() const
{
    return *llvm_module_;
}

std::unique_ptr<llvm::Module> module::detach_module()
{
    return std::move(llvm_module_);
}

compiler::compiler() : ctx_(std::make_shared<llvm::LLVMContext>()), current_module_(nullptr), env_(*this), compilation_ctx_(nullptr)
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

void compiler::compile(const qubus::function& func)
{
    current_module_->create_function(func, *this);
}

reference compiler::materialize_object(reference obj_id)
{
    auto& builder = env().builder();

    llvm::Type* obj_id_type;

    auto prefix = builder.CreateStructGEP(obj_id_type, obj_id.addr(), 0, "prefix");

    auto is_local_prefix = builder.CreateICmpEQ(prefix, builder.getIntN(64, 0));

    auto result = create_entry_block_alloca(env().get_current_function(), obj_id_type, nullptr, "local_id");

    auto result_ref = reference(result, access_path(), obj_id.datatype());

    emit_if_else(reference(is_local_prefix, access_path(), types::bool_{}), [&] {
            auto id = load_from_ref(obj_id, env(), compiler_ctx());

            store_to_ref(result_ref, id, env(), compiler_ctx());
      }, [&]{
            // TODO: Call the runtime to materialize the object or error out if this
            // is not supported.
      }, env(), compiler_ctx());

    return result_ref;
}

reference compiler::get_sizeof(const type& t)
{
    auto& rt_library = env().rt_library();
}

reference compiler::compile_root_skeleton(const expression& root_skel)
{
    return compile(root_skel);
}

void compiler::start(std::string_view name)
{
    current_module_ = std::make_unique<module>(name, ctx_);

    compilation_ctx_ = std::make_unique<compilation_context>(env_);
}

std::unique_ptr<module> compiler::finalize()
{
    ctx_.reset();

    return std::move(current_module_);
}

llvm::LLVMContext& compiler::get_context()
{
    return *ctx_;
}

module& compiler::get_module()
{
    return *current_module_;
}

llvm_environment& compiler::env()
{
    return env_;
}

compilation_context& compiler::compiler_ctx()
{
    return *compilation_ctx_;
}

std::unique_ptr<module> compile(std::unique_ptr<::qubus::module> mod, compiler& comp)
{
    comp.start("Qubus module");

    for (const auto& func : mod->functions())
    {
        comp.compile(func);
    }

    return comp.finalize();
}
} // namespace jit
} // namespace qubus
