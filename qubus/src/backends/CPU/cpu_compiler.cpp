#include <qbb/qubus/backends/cpu_compiler.hpp>

#include <qbb/qubus/loop_optimizer.hpp>
#include <qbb/qubus/make_implicit_conversions_explicit.hpp>

// Workaround for LLVM's definition of DEBUG
#pragma push_macro("DEBUG")
#undef DEBUG

#include <llvm/ExecutionEngine/JITEventListener.h>

#include <llvm/Support/TargetSelect.h>
#include <llvm/Analysis/TargetTransformInfo.h>

#include <llvm/Transforms/Utils/Cloning.h>

#pragma pop_macro("DEBUG")

#include <qbb/qubus/jit/jit_engine.hpp>
#include <qbb/qubus/jit/optimization_pipeline.hpp>

#include <llvm/IR/Verifier.h>
#include <llvm/IR/DataLayout.h>

#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/FormattedStream.h>

#include <qbb/qubus/jit/cpuinfo.hpp>
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

        auto available_features = get_host_cpu_features();

        builder.setMCPU(llvm::sys::getHostCPUName());

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
