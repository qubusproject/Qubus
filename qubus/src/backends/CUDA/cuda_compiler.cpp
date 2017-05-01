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

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/jit/compiler.hpp>
#include <qubus/jit/control_flow.hpp>
#include <qubus/jit/entry_block_alloca.hpp>
#include <qubus/jit/execution_stack.hpp>
#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/load_store.hpp>
#include <qubus/jit/loops.hpp>
#include <qubus/jit/optimization_pipeline.hpp>

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Verifier.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <llvm/Target/TargetMachine.h>

#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>

#include <cstdlib>
#include <utility>

namespace qubus
{

namespace
{

llvm::Value* get_thread_id_x(llvm::IRBuilder<>& builder, llvm::Module& mod)
{
    auto fn_type = llvm::FunctionType::get(builder.getInt32Ty(), false);

    auto getter = mod.getOrInsertFunction("llvm.nvvm.read.ptx.sreg.tid.x", fn_type);

    return builder.CreateCall(getter);
}

llvm::Value* get_block_id_x(llvm::IRBuilder<>& builder, llvm::Module& mod)
{
    auto fn_type = llvm::FunctionType::get(builder.getInt32Ty(), false);

    auto getter = mod.getOrInsertFunction("llvm.nvvm.read.ptx.sreg.ctaid.x", fn_type);

    return builder.CreateCall(getter);
}

llvm::Value* get_block_dim_x(llvm::IRBuilder<>& builder, llvm::Module& mod)
{
    auto fn_type = llvm::FunctionType::get(builder.getInt32Ty(), false);

    auto getter = mod.getOrInsertFunction("llvm.nvvm.read.ptx.sreg.ntid.x", fn_type);

    return builder.CreateCall(getter);
}

llvm::Value* get_grid_dim_x(llvm::IRBuilder<>& builder, llvm::Module& mod)
{
    auto fn_type = llvm::FunctionType::get(builder.getInt32Ty(), false);

    auto getter = mod.getOrInsertFunction("llvm.nvvm.read.ptx.sreg.nctaid.x", fn_type);

    return builder.CreateCall(getter);
}

llvm::Function* get_div_ceil32(llvm::IRBuilder<>& builder, llvm::Module& mod)
{
    if (auto round_up = mod.getFunction("qubus_div_ceil32"))
        return round_up;

    auto old_basic_block = builder.GetInsertBlock();
    auto old_insert_point = builder.GetInsertPoint();

    llvm::FunctionType* func_type = llvm::FunctionType::get(
        builder.getInt32Ty(), {builder.getInt32Ty(), builder.getInt32Ty()}, false);

    llvm::Function* round_up =
        llvm::Function::Create(func_type, llvm::Function::InternalLinkage, "qubus_div_ceil32", &mod);

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(mod.getContext(), "entry", round_up);
    builder.SetInsertPoint(entry);

    auto arg_iter = round_up->arg_begin();

    llvm::Value* lhs = &*arg_iter;
    ++arg_iter;
    llvm::Value* rhs = &*arg_iter;

    auto guard_cond = builder.CreateICmpEQ(lhs, builder.getInt32(0));

    llvm::BasicBlock* zero = llvm::BasicBlock::Create(mod.getContext(), "zero", round_up);
    llvm::BasicBlock* base = llvm::BasicBlock::Create(mod.getContext(), "base", round_up);

    builder.CreateCondBr(guard_cond, zero, base);

    builder.SetInsertPoint(zero);

    builder.CreateRet(builder.getInt32(0));

    builder.SetInsertPoint(base);

    auto shifted_lhs = builder.CreateSub(lhs, builder.getInt32(1), "shifted_lhs", true, true);
    auto div_down = builder.CreateUDiv(shifted_lhs, rhs, "div_down");
    auto result = builder.CreateAdd(div_down, builder.getInt32(1), "result", true, true);

    builder.SetInsertPoint(old_basic_block, old_insert_point);

    return round_up;
}

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
            env.ctx(),
            {md_builder.createConstant(kernel), md_builder.createString("kernel"),
             md_builder.createConstant(builder.getInt32(1))});

        kernel_annotations->addOperand(kernel_annotation);

        return kernel;
    }

    jit::reference compile_root_skeleton(const expression& root_skel) override
    {
        using pattern::_;

        pattern::variable<std::vector<std::reference_wrapper<const expression>>> tasks;
        pattern::variable<variable_declaration> index;
        pattern::variable<const expression&> lower_bound, upper_bound, increment, body;

        auto m =
            pattern::make_matcher<expression, jit::reference>()
                .case_(
                    unordered_for(index, lower_bound, upper_bound, increment, body),
                    [&] {
                        // Distribute all sub-tasks onto the available threads via a grid-stride loop.

                        auto& mod = this->get_module();
                        auto& ctx = mod.ctx();
                        auto& env = mod.env();

                        auto& builder = env.builder();
                        auto& symbol_table = ctx.symbol_table();

                        auto size_type = env.map_qubus_type(types::integer());

                        auto virtual_thread_id = jit::create_entry_block_alloca(
                            env.get_current_function(), size_type, nullptr, "vtid");

                        auto virtual_thread_id_ref =
                            jit::reference(virtual_thread_id, jit::access_path(), types::integer());

                        auto increment_ptr = compile(increment.get());
                        auto increment_value = jit::load_from_ref(increment_ptr, env, ctx);

                        auto lower_bound_ptr = compile(lower_bound.get());
                        auto upper_bound_ptr = compile(upper_bound.get());

                        auto lower_bound_value = load_from_ref(lower_bound_ptr, env, ctx);
                        auto upper_bound_value = load_from_ref(upper_bound_ptr, env, ctx);

                        auto thread_id = get_thread_id_x(builder, env.module());
                        auto block_id = get_block_id_x(builder, env.module());
                        auto block_dim = get_block_dim_x(builder, env.module());
                        auto grid_dim = get_grid_dim_x(builder, env.module());

                        // gtid = block_id * block_dim + thread_id
                        auto block_start_id =
                            builder.CreateMul(block_id, block_dim, "block_start", true, true);
                        auto global_thread_id =
                            builder.CreateAdd(block_start_id, thread_id, "gtid", true, true);

                        auto number_of_iterations = builder.CreateSub(
                            upper_bound_value, lower_bound_value, "num_iter", true, true);
                        auto div_ceil32 = get_div_ceil32(builder, env.module());
                        auto number_of_virtual_threads =
                            builder.CreateCall(div_ceil32, {number_of_iterations, increment_value});

                        auto grid_size =
                            builder.CreateMul(block_dim, grid_dim, "grid_size", true, true);

                        // Emit the grid-stride loop
                        jit::emit_loop(virtual_thread_id_ref, global_thread_id,
                                       number_of_virtual_threads, grid_size,
                                       [&] {
                                           auto virtual_thread_id_value =
                                               jit::load_from_ref(virtual_thread_id_ref, env, ctx);

                                           auto offset = builder.CreateMul(increment_value,
                                                                           virtual_thread_id_value,
                                                                           "offset", true, true);

                                           auto induction_var = builder.CreateAdd(
                                               offset, lower_bound_value, "inv_var", true, true);

                                           jit::reference induction_var_ref(
                                               induction_var, jit::access_path(), types::integer());

                                           symbol_table[index.get().id()] = induction_var_ref;

                                           compile(body.get());
                                       },
                                       env, ctx);

                        return jit::reference();
                    })
                .case_(_, [&](const expression& self) {
                    // Serialize the execution if we have not found a parallel loop.

                    auto& mod = this->get_module();
                    auto& ctx = mod.ctx();
                    auto& env = mod.env();

                    auto& builder = env.builder();

                    auto thread_id = get_thread_id_x(builder, env.module());
                    auto block_id = get_block_id_x(builder, env.module());
                    auto block_dim = get_block_dim_x(builder, env.module());

                    // gtid = block_id * block_dim + thread_id
                    auto block_start_id =
                        builder.CreateMul(block_id, block_dim, "block_start", true, true);
                    auto global_thread_id =
                        builder.CreateAdd(block_start_id, thread_id, "gtid", true, true);

                    auto cond = builder.CreateICmpEQ(global_thread_id, builder.getInt32(0));

                    jit::reference guard(cond, jit::access_path(), types::bool_{});

                    jit::emit_if_else(guard, [&] { compile(self); }, [] {}, env, ctx);

                    return jit::reference();
                });

        return pattern::match(root_skel, m);
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
