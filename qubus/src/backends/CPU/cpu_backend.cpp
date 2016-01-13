#include <hpx/config.hpp>

#include <qbb/qubus/backends/cpu_backend.hpp>

#include <qbb/qubus/backend.hpp>
#include <qbb/qubus/host_backend.hpp>
#include <qbb/qubus/compiler.hpp>

#include <qbb/qubus/backends/cpuinfo.hpp>

#include <qbb/qubus/backends/cpu_allocator.hpp>
#include <qbb/qubus/backends/cpu_memory_block.hpp>
#include <qbb/qubus/local_address_space.hpp>

#include <qbb/qubus/backends/cpu_object_factory.hpp>

#include <qbb/qubus/abi_info.hpp>

#include <qbb/qubus/metadata_builder.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <qbb/qubus/IR/type_inference.hpp>

#include <qbb/util/make_unique.hpp>

#include <qubus/qbb_qubus_export.h>

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

#include <qbb/qubus/jit/llvm_environment.hpp>
#include <qbb/qubus/jit/compiler.hpp>
#include <qbb/qubus/jit/execution_stack.hpp>

#include <hpx/async.hpp>

#include <hpx/lcos/local/promise.hpp>
#include <hpx/lcos/future.hpp>
#include <hpx/lcos/wait_all.hpp>

#include <boost/optional.hpp>
#include <boost/signals2.hpp>

#include <qbb/util/optional_ref.hpp>
#include <qbb/util/make_unique.hpp>
#include <qbb/util/assert.hpp>
#include <qbb/util/unused.hpp>

#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <algorithm>
#include <vector>

namespace qbb
{
namespace qubus
{

class cpu_runtime
{
public:
    cpu_runtime() : scratch_mem_(8 * 1024 * 1024), current_stack_ptr_(scratch_mem_.data())
    {
    }

    void* alloc_scratch_mem(util::index_t size)
    {
        void* addr = current_stack_ptr_;

        current_stack_ptr_ += size;

        return addr;
    }

    void dealloc_scratch_mem(util::index_t size)
    {
        current_stack_ptr_ -= size;
    }

private:
    std::vector<char> scratch_mem_;
    char* current_stack_ptr_;
};

extern "C" QBB_QUBUS_EXPORT void* qbb_qubus_cpurt_alloc_scatch_mem(cpu_runtime* runtime,
                                                                   util::index_t size)
{
    return runtime->alloc_scratch_mem(size);
}

extern "C" QBB_QUBUS_EXPORT void qbb_qubus_cpurt_dealloc_scratch_mem(cpu_runtime* runtime,
                                                                     util::index_t size)
{
    runtime->dealloc_scratch_mem(size);
}

namespace
{

class cpu_plan
{
public:
    explicit cpu_plan(std::function<void(void* const*, void*)> entry_) : entry_(std::move(entry_))
    {
    }

    cpu_plan(const cpu_plan&) = delete;
    cpu_plan& operator=(const cpu_plan&) = delete;

    void execute(const std::vector<void*>& args, cpu_runtime& runtime) const
    {
        entry_(args.data(), &runtime);
    }

private:
    std::function<void(void* const*, void*)> entry_;
};

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

class cpu_plan_registry
{
public:
    plan register_plan(std::unique_ptr<cpu_plan> p, std::vector<intent> intents)
    {
        auto plan_handle = handle_fac_.create();

        plans_.emplace(plan_handle, std::move(p));

        return plan(plan_handle, std::move(intents));
    }

    const cpu_plan& lookup_plan(const plan& handle) const
    {
        return *plans_.at(handle.id());
    }

private:
    util::handle_factory handle_fac_;
    std::unordered_map<util::handle, std::unique_ptr<cpu_plan>> plans_;
};
}

class cpu_compiler : public compiler
{
public:
    explicit cpu_compiler(cpu_plan_registry& plan_registry_) : plan_registry_(&plan_registry_)
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

    virtual ~cpu_compiler() = default;

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

class cpu_executor : public executor
{
public:
    cpu_executor(cpu_plan_registry& plan_registry_, local_address_space& addr_space_,
                 const abi_info& abi_)
    : plan_registry_(&plan_registry_), addr_space_(&addr_space_), abi_(&abi_), exec_stack_(4 * 1024)
    {
    }

    virtual ~cpu_executor() = default;

    hpx::lcos::future<void> execute_plan(const plan& executed_plan, execution_context ctx) override
    {
        const cpu_plan& executed_cpu_plan = plan_registry_->lookup_plan(executed_plan);

        std::vector<void*> plan_args;

        std::vector<std::shared_ptr<memory_block>> used_mem_blocks;

        for (const auto& arg : ctx.args())
        {
            plan_args.push_back(
                build_object_metadata(*arg, *addr_space_, *abi_, exec_stack_, used_mem_blocks));
        }

        return hpx::async([&executed_cpu_plan, plan_args, used_mem_blocks, this]
                          {
                              cpu_runtime runtime;

                              executed_cpu_plan.execute(plan_args, runtime);
                              exec_stack_.clear();
                          });
    }

private:
    cpu_plan_registry* plan_registry_;
    local_address_space* addr_space_;
    const abi_info* abi_;
    execution_stack exec_stack_;
};

class cpu_backend final : public host_backend
{
public:
    cpu_backend(const abi_info& abi_)
    : addr_space_(util::make_unique<local_address_space>(util::make_unique<cpu_allocator>())),
      obj_factory_(util::make_unique<cpu_object_factory>(addr_space_->get_allocator(), abi_)),
      compiler_(util::make_unique<cpu_compiler>(plan_registry_)),
      executor_(util::make_unique<cpu_executor>(plan_registry_, *addr_space_, abi_))
    {
        // use hwloc to obtain informations over all local CPUs
    }

    virtual ~cpu_backend() = default;

    std::string id() const override
    {
        return "qubus.cpu";
    }

    std::vector<executor*> executors() const override
    {
        return {executor_.get()};
    }

    compiler& get_compiler() const override
    {
        return *compiler_;
    }

    local_object_factory& local_factory() const override
    {
        return *obj_factory_;
    }

    local_address_space& address_space() const override
    {
        return *addr_space_;
    }

    plan register_function_as_plan(std::function<void(void* const*)> func,
                                   std::vector<intent> intents) override
    {
        auto thunk = [func = std::move(func)](void* const* args, void* QBB_UNUSED(runtime))
        {
            func(args);
        };

        return plan_registry_.register_plan(util::make_unique<cpu_plan>(std::move(thunk)),
                                            std::move(intents));
    }

private:
    std::unique_ptr<local_address_space> addr_space_;
    std::unique_ptr<cpu_object_factory> obj_factory_;
    cpu_plan_registry plan_registry_;
    std::unique_ptr<cpu_compiler> compiler_;
    std::unique_ptr<cpu_executor> executor_;
};

extern "C" QBB_QUBUS_EXPORT unsigned long int cpu_backend_get_api_version()
{
    return 0;
}

std::unique_ptr<cpu_backend> the_cpu_backend;
std::once_flag cpu_backend_init_flag;

extern "C" QBB_QUBUS_EXPORT backend* init_cpu_backend(const abi_info* abi)
{
    std::call_once(cpu_backend_init_flag, [&]
                   {
                       the_cpu_backend = util::make_unique<cpu_backend>(*abi);
                   });

    return the_cpu_backend.get();
}
}
}
