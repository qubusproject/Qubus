#ifndef QBB_QUBUS_JIT_COMPILER_HPP
#define QBB_QUBUS_JIT_COMPILER_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>

#include <qbb/qubus/jit/compilation_context.hpp>
#include <qbb/qubus/jit/llvm_environment.hpp>
#include <qbb/qubus/jit/reference.hpp>

#include <llvm/IR/LLVMContext.h>

#include <memory>
#include <string>

namespace qbb
{
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

    virtual void compile_entry_function(const function_declaration& func);

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
}

#endif
