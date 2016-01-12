#ifndef QBB_QUBUS_JIT_COMPILER_HPP
#define QBB_QUBUS_JIT_COMPILER_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>

#include <qbb/qubus/jit/compilation_context.hpp>
#include <qbb/qubus/jit/llvm_environment.hpp>
#include <qbb/qubus/jit/reference.hpp>

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
    explicit module(std::string namespace_);

    const std::string& get_namespace() const;

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
    compiler() = default;

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

    virtual void reset();
private:
    module* current_module_;
};

std::unique_ptr<module> compile(const function_declaration& func, compiler& comp);

}
}
}

#endif
