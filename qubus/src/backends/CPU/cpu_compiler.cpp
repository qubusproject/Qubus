#include <qbb/qubus/backends/cpu_compiler.hpp>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/JITEventListener.h>

#include <llvm/Support/TargetSelect.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

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

namespace qbb
{
namespace qubus
{

namespace
{
std::unique_ptr<cpu_plan> compile(function_declaration entry_point, llvm::ExecutionEngine& engine)
{
    jit::compiler comp;

    auto mod = jit::compile(entry_point, comp);

    std::unique_ptr<llvm::Module> the_module = mod->env().detach_module();

    the_module->setDataLayout(engine.getDataLayout());

    llvm::verifyModule(*the_module);

    // the_module->dump();
    // std::cout << std::endl;

    llvm::legacy::FunctionPassManager fn_pass_man(the_module.get());
    llvm::legacy::PassManager pass_man;

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
    llvm::formatted_raw_ostream fm3log(m3log);

    llvm::PassManager pMPasses;
    // pMPasses.add(new llvm::DataLayoutPass(*engine.getDataLayout()));
    engine.getTargetMachine()->addPassesToEmitFile(pMPasses, fm3log,
                                                   llvm::TargetMachine::CGFT_AssemblyFile);
    pMPasses.run(*the_module);*/

    engine.finalizeObject();

    engine.addModule(std::move(the_module));

    using entry_t = void (*)(void* const*, void*);

    return util::make_unique<cpu_plan>(
        reinterpret_cast<entry_t>(engine.getFunctionAddress(mod->get_namespace())));
}
}

class cpu_compiler_impl : public compiler
{
public:
    explicit cpu_compiler_impl(cpu_plan_registry& plan_registry_) : plan_registry_(&plan_registry_)
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();

        llvm::EngineBuilder builder(
            util::make_unique<llvm::Module>("dummy", llvm::getGlobalContext()));

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

        engine_ = std::unique_ptr<llvm::ExecutionEngine>(builder.create());

#if LLVM_USE_INTEL_JITEVENTS
        llvm::JITEventListener* vtuneProfiler =
            llvm::JITEventListener::createIntelJITEventListener();
        engine_->RegisterJITEventListener(vtuneProfiler);
#endif
    }

    virtual ~cpu_compiler_impl() = default;

    plan compile_plan(const function_declaration& plan_decl) override
    {
        auto param_count = plan_decl.params().size();

        std::vector<intent> intents(param_count, intent::in);
        intents.push_back(intent::inout);

        auto compiled_plan = compile(plan_decl, *engine_);

        return plan_registry_->register_plan(std::move(compiled_plan), std::move(intents));
    }

private:
    std::unique_ptr<llvm::ExecutionEngine> engine_;
    cpu_plan_registry* plan_registry_;
};

cpu_compiler::cpu_compiler(cpu_plan_registry& plan_registry_)
: impl_(std::make_unique<cpu_compiler_impl>(plan_registry_))
{
}

plan cpu_compiler::compile_plan(const function_declaration& plan_decl)
{
    return impl_->compile_plan(plan_decl);
}
}
}
