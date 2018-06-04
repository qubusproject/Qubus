#include <qubus/jit/jit_engine.hpp>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>

#include <llvm/IR/Mangler.h>
#include <llvm/Support/DynamicLibrary.h>

namespace qubus
{

jit_engine::jit_engine(std::unique_ptr<llvm::TargetMachine> target_machine_)
: target_machine_(std::move(target_machine_)),
  data_layout_(get_target_machine().createDataLayout()),
  object_layer_([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
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
    auto resolver = llvm::orc::createLambdaResolver(
        [&](const std::string& Name) {
            if (auto Sym = compile_layer_.findSymbol(Name, false))
                return Sym;
            return llvm::JITSymbol(nullptr);
        },
        [](const std::string& Name) {
            if (auto SymAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(Name))
                return llvm::JITSymbol(SymAddr, llvm::JITSymbolFlags::Exported);
            return llvm::JITSymbol(nullptr);
        });

    auto handle = compile_layer_.addModule(std::move(module), std::move(resolver));

    return cantFail(std::move(handle));
}

void jit_engine::remove_module(module_handle_type handle)
{
    cantFail(compile_layer_.removeModule(handle));
}

llvm::JITSymbol jit_engine::find_symbol(const std::string& name)
{
    std::string mangled_name;
    llvm::raw_string_ostream mangled_name_stream(mangled_name);
    llvm::Mangler::getNameWithPrefix(mangled_name_stream, name, data_layout_);

    return compile_layer_.findSymbol(mangled_name_stream.str(), true);
}

std::string jit_engine::mangle(const std::string& name)
{
    std::string mangled_name;
    {
        llvm::raw_string_ostream mangled_name_stream(mangled_name);
        llvm::Mangler::getNameWithPrefix(mangled_name_stream, name, data_layout_);
    }
    return mangled_name;
}

}
