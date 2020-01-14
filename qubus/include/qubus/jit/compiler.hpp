#ifndef QUBUS_JIT_COMPILER_HPP
#define QUBUS_JIT_COMPILER_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/module.hpp>
#include <qubus/IR/function.hpp>

#include <qubus/jit/compilation_context.hpp>
#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/reference.hpp>

#include <qubus/object_id.hpp>

#include <llvm/IR/LLVMContext.h>

#include <memory>
#include <string>
#include <string_view>

namespace qubus
{
namespace jit
{

class compiler;

class function
{
public:
    explicit function(const qubus::function& definition_, llvm::Function& code_);

    const qubus::function& definition() const
    {
        return *definition_;
    }

    llvm::Function& code() const
    {
        return *code_;
    }
private:
    const qubus::function* definition_;
    llvm::Function* code_;
};

class exported_function
{
public:
    explicit exported_function(const qubus::jit::function& backing_function_, llvm::Function& thunk_)
    : backing_function_(&backing_function_), thunk_(&thunk_)
    {
    }

private:
    const qubus::jit::function* backing_function_;
    llvm::Function* thunk_;
};

class user_defined_type
{
public:
    explicit user_defined_type(types::struct_ definition_, llvm::Type& realization_);
private:
    types::struct_ definition_;
    llvm::Type* realization_;

    qubus::jit::function constructor_;
    runtime_function sizeof__;
    qubus::jit::function destructor_;
};

class module
{
public:
    explicit module(std::string_view name_, std::shared_ptr<llvm::LLVMContext> ctx_);
    std::unique_ptr<llvm::Module> detach_module();

    qubus::jit::function& create_function(const qubus::function& func, compiler& comp);
    qubus::jit::exported_function& created_exported_function(const qubus::function& func, compiler& comp);
    qubus::jit::exported_function& created_exported_function(const qubus::jit::function& func, compiler& comp);
    qubus::jit::user_defined_type& create_user_defined_type(types::struct_ type_definition, compiler& comp);

    llvm::Module& llvm_module();
    const llvm::Module& llvm_module() const;
private:
    std::shared_ptr<llvm::LLVMContext> ctx_;
    std::unique_ptr<llvm::Module> llvm_module_;

    std::vector<qubus::jit::function> functions_;
    std::vector<qubus::jit::exported_function> exported_functions_;
    std::vector<qubus::jit::user_defined_type> user_defined_types_;
};

class compiler
{
public:
    compiler();

    virtual ~compiler() = default;

    compiler(const compiler&) = delete;
    compiler& operator=(const compiler&) = delete;
    compiler(compiler&&) = delete;
    compiler& operator=(compiler&&) = delete;

    virtual reference compile(const expression& expr);
    virtual void compile(const qubus::function& func);

    virtual reference materialize_object(reference obj_id);

    virtual reference get_sizeof(const type& t);

    virtual reference compile_root_skeleton(const expression& root_skel);

    virtual std::string mangle_function_name(const symbol_id& func_name);
    virtual std::string mangle_foreign_function_name(const symbol_id& func_name);

    virtual void start(std::string_view name);
    virtual std::unique_ptr<module> finalize();

    llvm::LLVMContext& get_context();
    module& get_module();
    llvm_environment& env();
    compilation_context& compiler_ctx();
private:
    std::shared_ptr<llvm::LLVMContext> ctx_;
    std::unique_ptr<module> current_module_;
    llvm_environment env_;
    std::unique_ptr<compilation_context> compilation_ctx_;
};

std::unique_ptr<module> compile(std::unique_ptr<::qubus::module> mod, compiler& comp);

}
}

#endif
