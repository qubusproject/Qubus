#include <qbb/qubus/backends/cpu_compiler.hpp>

#include <qbb/qubus/loop_optimizer.hpp>
#include <qbb/qubus/make_implicit_conversions_explicit.hpp>

// Workaround for LLVM's definition of DEBUG
#pragma push_macro("DEBUG")
#undef DEBUG

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/ExecutionEngine/JITEventListener.h>

#include <llvm/Support/TargetSelect.h>
#include <llvm/Analysis/TargetTransformInfo.h>

#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#pragma pop_macro("DEBUG")

#include <qbb/qubus/jit/optimization_pipeline.hpp>

#include <llvm/IR/Verifier.h>
#include <llvm/IR/DataLayout.h>

#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>

#include <qbb/qubus/backends/cpuinfo.hpp>
#include <qbb/qubus/jit/llvm_environment.hpp>
#include <qbb/qubus/jit/compiler.hpp>
#include <qbb/qubus/jit/execution_stack.hpp>

#include <utility>
#include <cstdlib>

namespace qbb
{
namespace qubus
{

namespace
{

class cpu_plan_impl : public cpu_plan
{
public:
    cpu_plan_impl(std::function<void(void const*, void*)> entry_,
                  std::unique_ptr<jit::module> module_)
    : entry_(std::move(entry_)), module_(std::move(module_))
    {
    }

    virtual ~cpu_plan_impl() = default;

    void execute(const std::vector<void*>& args, cpu_runtime& runtime) const override
    {
        entry_(args.data(), &runtime);
    }

private:
    std::function<void(void const*, void*)> entry_;
    std::unique_ptr<jit::module> module_;
};

class jit_engine
{
public:
    using object_layer_type = llvm::orc::ObjectLinkingLayer<>;
    using compile_layer_type = llvm::orc::IRCompileLayer<object_layer_type>;
    using module_handle_type = compile_layer_type::ModuleSetHandleT;

    explicit jit_engine(std::unique_ptr<llvm::TargetMachine> target_machine_)
    : target_machine_(std::move(target_machine_)),
      data_layout_(get_target_machine().createDataLayout()),
      compile_layer_(object_layer_, llvm::orc::SimpleCompiler(get_target_machine()))
    {
        llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
    }

    llvm::TargetMachine& get_target_machine()
    {
        return *target_machine_;
    }

    module_handle_type add_module(std::unique_ptr<llvm::Module> module)
    {
#if LLVM_VERSION_MAJOR < 4
        auto resolver = llvm::orc::createLambdaResolver(
            [&](const std::string& name)
            {
                if (auto symbol = find_mangled_symbol(name))
                    return llvm::RuntimeDyld::SymbolInfo(symbol.getAddress(), symbol.getFlags());
                return llvm::RuntimeDyld::SymbolInfo(nullptr);
            },
            [](const std::string& S)
            {
                return nullptr;
            });
#else
        auto resolver = llvm::orc::createLambdaResolver(
                [&](const std::string& name)
                {
                    if (auto symbol = find_mangled_symbol(name))
                        return symbol;
                    return llvm::JITSymbol(nullptr);
                },
                [](const std::string& S)
                {
                    return nullptr;
                });
#endif

        auto handle = compile_layer_.addModuleSet(make_singleton_set(std::move(module)),
                                                  std::make_unique<llvm::SectionMemoryManager>(),
                                                  std::move(resolver));

        module_handles_.push_back(handle);
        return handle;
    }

    void remove_module(module_handle_type handle)
    {
        module_handles_.erase(std::find(module_handles_.begin(), module_handles_.end(), handle));
        compile_layer_.removeModuleSet(handle);
    }

#if LLVM_VERSION_MAJOR < 4
    llvm::orc::JITSymbol find_symbol(const std::string name)
    {
        return find_mangled_symbol(mangle(name));
    }
#else
    llvm::JITSymbol find_symbol(const std::string name)
    {
        return find_mangled_symbol(mangle(name));
    }
#endif

private:
    std::string mangle(const std::string& name)
    {
        std::string mangled_name;
        {
            llvm::raw_string_ostream mangled_name_stream(mangled_name);
            llvm::Mangler::getNameWithPrefix(mangled_name_stream, name, data_layout_);
        }
        return mangled_name;
    }

    template <typename T>
    static std::vector<T> make_singleton_set(T t)
    {
        std::vector<T> vec;
        vec.push_back(std::move(t));
        return vec;
    }

#if LLVM_VERSION_MAJOR < 4
    llvm::orc::JITSymbol find_mangled_symbol(const std::string& name)
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
    llvm::JITSymbol find_mangled_symbol(const std::string& name)
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

    std::unique_ptr<llvm::TargetMachine> target_machine_;
    llvm::DataLayout data_layout_;
    object_layer_type object_layer_;
    compile_layer_type compile_layer_;
    std::vector<module_handle_type> module_handles_;
};

std::unique_ptr<cpu_plan> compile(function_declaration entry_point, jit::compiler& comp,
                                  jit_engine& engine)
{
    auto mod = jit::compile(entry_point, comp);

    std::unique_ptr<llvm::Module> the_module = llvm::CloneModule(&mod->env().module());

    the_module->setDataLayout(engine.get_target_machine().createDataLayout());
    the_module->setTargetTriple(engine.get_target_machine().getTargetTriple().getTriple());

    llvm::verifyModule(*the_module);

    // the_module->dump();
    // std::cout << std::endl;

    llvm::legacy::FunctionPassManager fn_pass_man(the_module.get());
    llvm::legacy::PassManager pass_man;

    auto& TM = engine.get_target_machine();

    fn_pass_man.add(llvm::createTargetTransformInfoWrapperPass(TM.getTargetIRAnalysis()));
    pass_man.add(llvm::createTargetTransformInfoWrapperPass(TM.getTargetIRAnalysis()));

    jit::setup_function_optimization_pipeline(fn_pass_man, true);
    jit::setup_optimization_pipeline(pass_man, true, true);

    fn_pass_man.doInitialization();

    for (auto& fn : *the_module)
    {
        fn_pass_man.run(fn);
    }

    fn_pass_man.doFinalization();

    pass_man.run(*the_module);

    // the_module->dump();
    // std::cout << std::endl;

    /*std::cout << "The assembler output:\n\n";

    llvm::raw_os_ostream m3log(std::cout);
    llvm::buffer_ostream fm3log(m3log);

    llvm::legacy::PassManager pMPasses;

    TM.addPassesToEmitFile(pMPasses, fm3log, llvm::TargetMachine::CGFT_AssemblyFile);
    pMPasses.run(*the_module);*/

    engine.add_module(std::move(the_module));

    using entry_t = void (*)(void const*, void*);

    auto entry = engine.find_symbol(mod->get_namespace());

    return util::make_unique<cpu_plan_impl>(reinterpret_cast<entry_t>(entry.getAddress()), std::move(mod));
}
}

class cpu_compiler_impl
{
public:
    explicit cpu_compiler_impl() : comp_(std::make_unique<jit::compiler>())
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();

        llvm::EngineBuilder builder;

        std::vector<std::string> available_features;
        llvm::StringMap<bool> features;

        if (llvm::sys::getHostCPUFeatures(features))
        {
            for (const auto& feature : features)
            {
                if (feature.getValue())
                {
                    available_features.push_back(feature.getKey());
                }
            }
        }
        else
        {
            builder.setMCPU(llvm::sys::getHostCPUName());

            available_features = deduce_host_cpu_features();
        }

        builder.setMAttrs(available_features);
        builder.setOptLevel(llvm::CodeGenOpt::Aggressive);

        llvm::TargetOptions options;
        options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
        options.UnsafeFPMath = 1;
        options.NoInfsFPMath = 1;
        options.NoNaNsFPMath = 1;

        builder.setTargetOptions(options);

        auto TM = std::unique_ptr<llvm::TargetMachine>(builder.selectTarget());

        engine_ = std::make_unique<jit_engine>(std::move(TM));

#if LLVM_USE_INTEL_JITEVENTS
// TODO: Reenable this
// llvm::JITEventListener* vtuneProfiler =
//    llvm::JITEventListener::createIntelJITEventListener();
// engine_->RegisterJITEventListener(vtuneProfiler);
#endif
    }

    std::unique_ptr<cpu_plan> compile_computelet(function_declaration computelet)
    {
        // Don't enable this line! The loop optimizer is broken.
        //computelet = optimize_loops(computelet);

        computelet = make_implicit_conversions_explicit(computelet);

        return compile(std::move(computelet), *comp_, *engine_);
    }

private:
    std::unique_ptr<jit::compiler> comp_;
    std::unique_ptr<jit_engine> engine_;
};

cpu_compiler::cpu_compiler() : impl_(std::make_unique<cpu_compiler_impl>())
{
}

std::unique_ptr<cpu_plan> cpu_compiler::compile_computelet(const function_declaration& computelet)
{
    return impl_->compile_computelet(computelet);
}
}
}
