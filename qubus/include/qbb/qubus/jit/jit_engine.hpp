#ifndef QUBUS_JIT_JIT_ENGINE_HPP
#define QUBUS_JIT_JIT_ENGINE_HPP

// Workaround for LLVM's definition of DEBUG
#pragma push_macro("DEBUG")
#undef DEBUG

#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>

#include <llvm/IR/Module.h>

#include <memory>
#include <string>
#include <vector>

inline namespace qbb
{
namespace qubus
{

class jit_engine
{
public:
    using object_layer_type = llvm::orc::ObjectLinkingLayer<>;
    using compile_layer_type = llvm::orc::IRCompileLayer<object_layer_type>;
    using module_handle_type = compile_layer_type::ModuleSetHandleT;

    explicit jit_engine(std::unique_ptr<llvm::TargetMachine> target_machine_);

    llvm::TargetMachine& get_target_machine();

    module_handle_type add_module(std::unique_ptr<llvm::Module> module);

    void remove_module(module_handle_type handle);

#if LLVM_VERSION_MAJOR < 4
    llvm::orc::JITSymbol find_symbol(const std::string name);
#else
    llvm::JITSymbol find_symbol(const std::string name);
#endif

private:
    std::string mangle(const std::string& name);

    template <typename T>
    static std::vector<T> make_singleton_set(T t)
    {
        std::vector<T> vec;
        vec.push_back(std::move(t));
        return vec;
    }

#if LLVM_VERSION_MAJOR < 4
    llvm::orc::JITSymbol find_mangled_symbol(const std::string& name);
#else
    llvm::JITSymbol find_mangled_symbol(const std::string& name);
#endif

    std::unique_ptr<llvm::TargetMachine> target_machine_;
    llvm::DataLayout data_layout_;
    object_layer_type object_layer_;
    compile_layer_type compile_layer_;
    std::vector<module_handle_type> module_handles_;
};
}
}

#pragma pop_macro("DEBUG")

#endif
