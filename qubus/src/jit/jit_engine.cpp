#include <qbb/qubus/jit/jit_engine.hpp>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>

#include <llvm/IR/Mangler.h>
#include <llvm/Support/DynamicLibrary.h>

namespace qubus
{

jit_engine::jit_engine(std::unique_ptr<llvm::TargetMachine> target_machine_)
: target_machine_(std::move(target_machine_)),
  data_layout_(get_target_machine().createDataLayout()),
  compile_layer_(object_layer_, llvm::orc::SimpleCompiler(get_target_machine()))
{
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

llvm::TargetMachine& jit_engine::get_target_machine()
{
    return *target_machine_;
}

jit_engine::module_handle_type jit_engine::add_module(std::unique_ptr<llvm::Module> module)
{
#if LLVM_VERSION_MAJOR < 4
    auto resolver = llvm::orc::createLambdaResolver(
        [&](const std::string& name) {
            if (auto symbol = find_mangled_symbol(name))
                return llvm::RuntimeDyld::SymbolInfo(symbol.getAddress(), symbol.getFlags());
            return llvm::RuntimeDyld::SymbolInfo(nullptr);
        },
        [](const std::string& S) { return nullptr; });
#else
    auto resolver = llvm::orc::createLambdaResolver(
        [&](const std::string& name) {
            if (auto symbol = find_mangled_symbol(name))
                return symbol;
            return llvm::JITSymbol(nullptr);
        },
        [](const std::string& S) { return nullptr; });
#endif

    auto handle = compile_layer_.addModuleSet(make_singleton_set(std::move(module)),
                                              std::make_unique<llvm::SectionMemoryManager>(),
                                              std::move(resolver));

    module_handles_.push_back(handle);
    return handle;
}

void jit_engine::remove_module(module_handle_type handle)
{
    module_handles_.erase(std::find(module_handles_.begin(), module_handles_.end(), handle));
    compile_layer_.removeModuleSet(handle);
}

#if LLVM_VERSION_MAJOR < 4
llvm::orc::JITSymbol jit_engine::find_symbol(const std::string name)
{
    return find_mangled_symbol(mangle(name));
}
#else
llvm::JITSymbol jit_engine::find_symbol(const std::string name)
{
    return find_mangled_symbol(mangle(name));
}
#endif

std::string jit_engine::mangle(const std::string& name)
{
    std::string mangled_name;
    {
        llvm::raw_string_ostream mangled_name_stream(mangled_name);
        llvm::Mangler::getNameWithPrefix(mangled_name_stream, name, data_layout_);
    }
    return mangled_name;
}

#if LLVM_VERSION_MAJOR < 4
llvm::orc::JITSymbol jit_engine::find_mangled_symbol(const std::string& name)
{
    for (auto handle : llvm::make_range(module_handles_.begin(), module_handles_.end()))
        if (auto symbol = compile_layer_.findSymbolIn(handle, name, true))
            return symbol;

    // If we can't find the symbol in the JIT, try looking in the host process.
    if (auto sym_addr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name))
        return llvm::orc::JITSymbol(sym_addr, llvm::JITSymbolFlags::Exported);

    return nullptr;
}
#else
llvm::JITSymbol jit_engine::find_mangled_symbol(const std::string& name)
{
    for (auto handle : llvm::make_range(module_handles_.begin(), module_handles_.end()))
        if (auto symbol = compile_layer_.findSymbolIn(handle, name, true))
            return symbol;

    // If we can't find the symbol in the JIT, try looking in the host process.
    if (auto sym_addr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name))
        return llvm::JITSymbol(sym_addr, llvm::JITSymbolFlags::Exported);

    return nullptr;
}
#endif
}
