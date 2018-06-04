#include <qubus/backends/cpu/cpu_compiler.hpp>

#include <qubus/loop_optimizer.hpp>
#include <qubus/make_implicit_conversions_explicit.hpp>

// Workaround for LLVM's definition of DEBUG
#pragma push_macro("DEBUG")
#undef DEBUG

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JITEventListener.h>

#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/Transforms/Utils/Cloning.h>

#pragma pop_macro("DEBUG")

#include <qubus/jit/jit_engine.hpp>
#include <qubus/jit/optimization_pipeline.hpp>

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Verifier.h>

#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/raw_os_ostream.h>

#include <llvm/Support/Host.h>

#include <qubus/jit/compiler.hpp>
#include <qubus/jit/cpuinfo.hpp>
#include <qubus/jit/execution_stack.hpp>
#include <qubus/jit/llvm_environment.hpp>

#include <cstdlib>
#include <utility>

namespace qubus
{

namespace
{

std::string mangle_function_name(const symbol_id& func_name)
{
    std::string result;

    for (auto iter = func_name.components().begin(), end = func_name.components().end(); iter != end; ++iter)
    {
        result += *iter;

        result += '_';
    }

    result += "ffi";

    return result;
}

class cpu_plan_impl final : public cpu_plan
{
public:
    cpu_plan_impl(jit_engine& jit_engine_, std::unique_ptr<jit::module> module_)
    : jit_engine_(&jit_engine_), module_(std::move(module_))
    {
    }

    void execute(const symbol_id& entry_point, const std::vector<void*>& args,
                 cpu_runtime& runtime) const override
    {
        using entry_t = void (*)(void const*, void*);

        auto entry = jit_engine_->find_symbol(mangle_function_name(entry_point));

        auto address = reinterpret_cast<entry_t>(cantFail(entry.getAddress()));

        QUBUS_ASSERT(address != nullptr, "Invalid address.");

        address(args.data(), &runtime);
    }

private:
    jit_engine* jit_engine_; // TODO: Turn this into a shared ptr or at least a weak ptr.
    std::unique_ptr<jit::module> module_;
};

std::unique_ptr<cpu_plan> compile(std::unique_ptr<module> program, jit::compiler& comp,
                                  jit_engine& engine)
{
    program = make_implicit_conversions_explicit(*program);

    auto mod = jit::compile(std::move(program), comp);

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
    jit::setup_optimization_pipeline(pass_man, true, false);

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

    return util::make_unique<cpu_plan_impl>(engine, std::move(mod));
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

    std::unique_ptr<cpu_plan> compile_computelet(std::unique_ptr<module> program)
    {
        return compile(std::move(program), *comp_, *engine_);
    }

private:
    std::unique_ptr<jit::compiler> comp_;
    std::unique_ptr<jit_engine> engine_;
};

cpu_compiler::cpu_compiler() : impl_(std::make_unique<cpu_compiler_impl>())
{
}

std::unique_ptr<cpu_plan> cpu_compiler::compile_computelet(std::unique_ptr<module> program)
{
    return impl_->compile_computelet(std::move(program));
}
}
