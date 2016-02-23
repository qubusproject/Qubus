#include <hpx/config.hpp>

#include <qbb/qubus/backends/cpu_backend.hpp>

#include <qbb/qubus/backend.hpp>
#include <qbb/qubus/host_backend.hpp>
#include <qbb/qubus/compiler.hpp>

#include <qbb/qubus/abi_info.hpp>
#include <qbb/qubus/metadata_builder.hpp>
#include <qbb/qubus/local_address_space.hpp>

#include <qbb/qubus/backends/cpu_plan_registry.hpp>
#include <qbb/qubus/backends/cpu_compiler.hpp>

#include <qbb/qubus/backends/cpu_allocator.hpp>
#include <qbb/qubus/backends/cpu_memory_block.hpp>
#include <qbb/qubus/backends/cpu_object_factory.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <qbb/qubus/IR/type_inference.hpp>

#include <qbb/util/make_unique.hpp>

#include <qubus/qbb_qubus_export.h>

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

class cpu_executor : public executor
{
public:
    cpu_executor(cpu_plan_registry& plan_registry_, const abi_info& abi_)
    : plan_registry_(&plan_registry_), abi_(&abi_), exec_stack_(4 * 1024)
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
            plan_args.push_back(build_object_metadata(*arg, *abi_, exec_stack_, used_mem_blocks));
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
    const abi_info* abi_;
    execution_stack exec_stack_;
};

class cpu_backend final : public host_backend
{
public:
    cpu_backend(const abi_info& abi_)
    : compiler_(util::make_unique<cpu_compiler>(plan_registry_)),
      executor_(util::make_unique<cpu_executor>(plan_registry_, abi_))
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
