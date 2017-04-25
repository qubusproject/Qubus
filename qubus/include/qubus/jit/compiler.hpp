#ifndef QUBUS_JIT_COMPILER_HPP
#define QUBUS_JIT_COMPILER_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function_declaration.hpp>

#include <qubus/jit/compilation_context.hpp>
#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/reference.hpp>

#include <llvm/IR/LLVMContext.h>

#include <memory>
#include <string>

namespace qubus
{
namespace jit
{

class module
{
public:
    module(std::string namespace_, llvm::LLVMContext& llvm_ctx_);

    const std::string& get_namespace() const;

    std::unique_ptr<llvm::Module> detach_module();

    llvm_environment& env();
    compilation_context& ctx();

    void finish();
private:
    std::string namespace_;
    llvm_environment env_;
    std::unique_ptr<compilation_context> ctx_;
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
    virtual void compile(const function_declaration& func);

    virtual llvm::Function* compile_entry_function(const function_declaration& func);
    virtual reference compile_root_skeleton(const expression& root_skel);

    void set_module(module& current_module);
    module& get_module();

    llvm::LLVMContext& get_context();

    virtual void reset();
private:
    std::unique_ptr<llvm::LLVMContext> ctx_;
    module* current_module_;
};

std::unique_ptr<module> compile(const function_declaration& func, compiler& comp);

}
}

#endif
