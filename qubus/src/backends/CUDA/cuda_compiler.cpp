#include <qubus/backends/cuda/cuda_compiler.hpp>

#include <qubus/loop_optimizer.hpp>
#include <qubus/make_implicit_conversions_explicit.hpp>

// Workaround for LLVM's definition of DEBUG
#pragma push_macro("DEBUG")
#undef DEBUG

#include <llvm/ExecutionEngine/JITEventListener.h>

#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/Transforms/Utils/Cloning.h>

#pragma pop_macro("DEBUG")

#include <qubus/jit/optimization_pipeline.hpp>

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Verifier.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <llvm/Target/TargetMachine.h>

#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>

#include <qubus/jit/compiler.hpp>
#include <qubus/jit/execution_stack.hpp>
#include <qubus/jit/llvm_environment.hpp>

#include <cstdlib>
#include <utility>

namespace qubus
{

namespace
{

class nvptx_compiler final : public jit::compiler
{
public:
    llvm::Function* compile_entry_function(const function_declaration& func) override
    {
        auto kernel = jit::compiler::compile_entry_function(func);

        auto& env = this->get_module().env();

        auto& builder = env.builder();
        auto& md_builder = env.md_builder();
        auto& module = env.module();

        auto kernel_annotations = module.getOrInsertNamedMetadata("nvvm.annotations");

        auto kernel_annotation = llvm::MDTuple::get(
            env.ctx(), {md_builder.createConstant(kernel), md_builder.createString("kernel"),
                        md_builder.createConstant(builder.getInt32(1))});

        kernel_annotations->addOperand(kernel_annotation);

        return kernel;
    }
};

class cuda_plan_impl : public cuda_plan
{
public:
    cuda_plan_impl(cuda::function entry_, std::unique_ptr<jit::module> module_)
    : entry_(std::move(entry_)), module_(std::move(module_))
    {
    }

    virtual ~cuda_plan_impl() = default;

    void execute(const std::vector<void*>& args, cuda::stream& stream) const override
    {
        auto launch_config = cuda::calculate_launch_config_with_max_occupancy(entry_, 0);

        cuda::launch_kernel(entry_, launch_config.min_grid_size, launch_config.block_size, 0,
                            stream, args.data(), nullptr);
    }

private:
    cuda::function entry_;
    std::unique_ptr<jit::module> module_;
};

std::unique_ptr<cuda_plan> compile(function_declaration entry_point, jit::compiler& comp,
                                   llvm::TargetMachine& target_machine)
{
    auto mod = jit::compile(entry_point, comp);

    std::unique_ptr<llvm::Module> the_module = llvm::CloneModule(&mod->env().module());

    the_module->setDataLayout(target_machine.createDataLayout());
    the_module->setTargetTriple(target_machine.getTargetTriple().getTriple());

    llvm::verifyModule(*the_module);

    the_module->dump();

    llvm::legacy::FunctionPassManager fn_pass_man(the_module.get());
    llvm::legacy::PassManager pass_man;

    fn_pass_man.add(
        llvm::createTargetTransformInfoWrapperPass(target_machine.getTargetIRAnalysis()));
    pass_man.add(llvm::createTargetTransformInfoWrapperPass(target_machine.getTargetIRAnalysis()));

    jit::setup_function_optimization_pipeline(fn_pass_man, true);
    jit::setup_optimization_pipeline(pass_man, true, true);

    llvm::SmallVector<char, 10> buffer;
    llvm::raw_svector_ostream sstream(buffer);

    if (target_machine.addPassesToEmitFile(
            pass_man, sstream, llvm::TargetMachine::CodeGenFileType::CGFT_AssemblyFile, true))
    {
        throw 0;
    }

    fn_pass_man.doInitialization();

    for (auto& fn : *the_module)
    {
        fn_pass_man.run(fn);
    }

    fn_pass_man.doFinalization();

    pass_man.run(*the_module);

    std::cout << buffer.data() << std::endl;

    cuda::device dev(0);
    cuda::context ctx(dev);

    cuda::module cuda_module(std::string(buffer.data()));

    auto entry = cuda_module.get_function(mod->get_namespace());

    return util::make_unique<cuda_plan_impl>(entry, std::move(mod));
}
}

class cuda_compiler::impl
{
public:
    explicit impl() : comp_(std::make_unique<nvptx_compiler>())
    {
        LLVMInitializeNVPTXTarget();
        LLVMInitializeNVPTXTargetInfo();
        LLVMInitializeNVPTXTargetMC();
        LLVMInitializeNVPTXAsmPrinter();

        cuda::init();

        llvm::EngineBuilder builder;

        builder.setOptLevel(llvm::CodeGenOpt::Aggressive);

        llvm::TargetOptions options;
        options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
        options.UnsafeFPMath = 1;
        options.NoInfsFPMath = 1;
        options.NoNaNsFPMath = 1;

        builder.setTargetOptions(options);

        llvm::Triple nvptx_triple("nvptx64-unknown-linux-gnu");

        llvm::SmallVector<std::string, 10> attrs;

        auto driver_version = cuda::get_driver_version();

        if (driver_version >= 8000)
        {
            attrs.push_back("ptx50");
        }
        else if (driver_version >= 7050)
        {
            attrs.push_back("ptx43");
        }
        else if (driver_version >= 7000)
        {
            attrs.push_back("ptx42");
        }
        else if (driver_version >= 6050)
        {
            attrs.push_back("ptx41");
        }
        else if (driver_version >= 6000)
        {
            attrs.push_back("ptx40");
        }
        else if (driver_version >= 5050)
        {
            attrs.push_back("ptx32");
        }
        else
        {
            throw 0; // invalid driver
        }

        target_machine_ = std::unique_ptr<llvm::TargetMachine>(
            builder.selectTarget(nvptx_triple, "", "sm_35", attrs));
    }

    std::unique_ptr<cuda_plan> compile_computelet(function_declaration computelet)
    {
        computelet = make_implicit_conversions_explicit(computelet);

        return compile(std::move(computelet), *comp_, *target_machine_);
    }

private:
    std::unique_ptr<nvptx_compiler> comp_;
    std::unique_ptr<llvm::TargetMachine> target_machine_;
};

cuda_compiler::cuda_compiler() : impl_(std::make_unique<cuda_compiler::impl>())
{
}

cuda_compiler::~cuda_compiler() = default;

std::unique_ptr<cuda_plan> cuda_compiler::compile_computelet(const function_declaration& computelet)
{
    return impl_->compile_computelet(computelet);
}
}
