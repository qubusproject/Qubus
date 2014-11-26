#include <hpx/config.hpp>

#include <qbb/kubus/runtime.hpp>

#include <qbb/kubus/backends/cpu_backend.hpp>

#include <qbb/kubus/plan.hpp>

#include <qbb/kubus/lower_top_level_sums.hpp>
#include <qbb/kubus/lower_abstract_indices.hpp>
#include <qbb/kubus/loop_optimizer.hpp>
#include <qbb/kubus/make_implicit_conversions_explicit.hpp>
#include <qbb/kubus/IR/pretty_printer.hpp>

#include <qbb/kubus/metadata_builder.hpp>
#include <qbb/kubus/backends/execution_stack.hpp>

#include <qbb/kubus/logging.hpp>

#include <boost/range/adaptor/indexed.hpp>

#include <qbb/util/get_prefix.hpp>
#include <qbb/util/make_unique.hpp>

#include <hpx/include/lcos.hpp>

#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <iterator>

namespace qbb
{
namespace kubus
{

namespace
{

class runtime_plan
{
public:
    virtual ~runtime_plan() = default;

    virtual hpx::lcos::future<void> execute(execution_context ctx) const = 0;
};

class simple_plan final : public runtime_plan
{
public:
    simple_plan(backend* backend_, plan handle_) : backend_(backend_), handle_(handle_)
    {
    }

    hpx::lcos::future<void> execute(execution_context ctx) const override
    {
        return backend_->executors()[0]->execute_plan(handle_, std::move(ctx));
    }

private:
    backend* backend_;
    plan handle_;
};

class user_defined_plan_executor
{
public:
    user_defined_plan_executor(local_address_space& addr_space_, const abi_info& abi_)
    : addr_space_(&addr_space_), abi_(&abi_), exec_stack_(1024)
    {
    }

    hpx::lcos::future<void> execute_plan(const user_defined_plan_body_t& body,
                                         const execution_context& ctx)
    {
        std::vector<void*> args;

        std::vector<std::shared_ptr<memory_block>> used_mem_blocks;

        for (const auto& arg : ctx.args())
        {
            args.push_back(
                build_object_metadata(*arg, *addr_space_, *abi_, exec_stack_, used_mem_blocks));
        }

        return hpx::async([body, args, used_mem_blocks]()
                          {
                              body(args.data());
                          });
    }

private:
    local_address_space* addr_space_;
    const abi_info* abi_;
    execution_stack exec_stack_;
};

class user_defined_plan final : public runtime_plan
{
public:
    explicit user_defined_plan(user_defined_plan_executor& executor_,
                               user_defined_plan_body_t body_)
    : executor_(&executor_), body_(std::move(body_))
    {
    }

    hpx::lcos::future<void> execute(execution_context ctx) const override
    {
        return executor_->execute_plan(body_, ctx);
    }

private:
    user_defined_plan_executor* executor_;
    user_defined_plan_body_t body_;
};
}

// TODO: Guard this with a mutex
class global_plan_repository
{
public:
    global_plan_repository(local_address_space& host_addr_space_, const abi_info& abi)
    : udp_executor_(host_addr_space_, abi)
    {
    }

    plan add_plan(backend* backend, plan handle)
    {
        auto id = id_factory_.create();

        plans_.emplace(id, util::make_unique<simple_plan>(backend, handle));

        return plan(id, handle.intents());
    }

    plan add_user_defined_plan(user_defined_plan_t p)
    {
        auto id = id_factory_.create();

        plans_.emplace(id, util::make_unique<user_defined_plan>(udp_executor_, std::move(p.body)));

        return plan(id, std::move(p.intents));
    }

    runtime_plan* lookup_plan(const plan& p) const
    {
        return plans_.at(p.id()).get();
    }

private:
    util::handle_factory id_factory_;
    std::unordered_map<util::handle, std::unique_ptr<runtime_plan>> plans_;
    user_defined_plan_executor udp_executor_;
};

runtime_executor::runtime_executor(global_plan_repository& plan_repository_)
: plan_repository_(&plan_repository_)
{
}

hpx::lcos::future<void> runtime_executor::execute_plan(const plan& executed_plan,
                                                       execution_context ctx)
{
    for (const auto& arg : ctx.args() | boost::adaptors::indexed())
    {
        bool must_alias = false;

        if (executed_plan.intents()[arg.index()] == intent::inout)
        {
            for (const auto& arg2 : ctx.args() | boost::adaptors::indexed())
            {
                if (arg.index() != arg2.index() && arg2.value()->id() == arg.value()->id())
                {
                    must_alias = true;
                    break;
                }
            }
        }

        if (must_alias)
        {
            /*
            auto original_result = ctx.result();

            auto temp_result = original_result->clone();

            auto patched_ctx = ctx;

            patched_ctx.set_result(temp_result);

            auto f = plan->execute(patched_ctx);

            return f.then([](const hpx::lcos::future<void>&)
            {
                original_result->substitute_with(temp_result);
            });
             */

            throw 0;
        }
    }

    auto plan = plan_repository_->lookup_plan(executed_plan);

    return plan->execute(std::move(ctx));
}

runtime::runtime()
{
    init_logging();

    BOOST_LOG_NAMED_SCOPE("runtime");

    logger slg;

    BOOST_LOG_SEV(slg, normal) << "Initialize the kubus runtime";

    BOOST_LOG_SEV(slg, normal) << "Runtime prefix: " << util::get_prefix("kubus");

    BOOST_LOG_SEV(slg, normal) << "Bootstrapping virtual multiprocessor";

    cpu_backend_ = init_cpu_backend(&abi_info_);

    plan_repository_ =
        util::make_unique<global_plan_repository>(cpu_backend_->address_space(), abi_info_);

    object_factory_.add_factory_and_addr_space(cpu_backend_->id(), cpu_backend_->local_factory(),
                                               cpu_backend_->address_space());

    runtime_exec_ = util::make_unique<runtime_executor>(*plan_repository_);

    scheduler_ = util::make_unique<greedy_scheduler>(*runtime_exec_);
}

runtime::~runtime()
{
}

object_factory& runtime::get_object_factory()
{
    return object_factory_;
}

plan runtime::compile(function_declaration decl)
{
    decl = lower_top_level_sums(decl);

    decl = optimize_loops(decl);

    decl = lower_abstract_indices(decl);

    decl = make_implicit_conversions_explicit(decl);

    auto& compiler = cpu_backend_->get_compiler();

    return plan_repository_->add_plan(cpu_backend_, compiler.compile_plan(decl));
}

plan runtime::register_user_defined_plan(user_defined_plan_t plan)
{
    return plan_repository_->add_user_defined_plan(std::move(plan));
}

void runtime::execute(plan p, execution_context ctx)
{
    scheduler_->schedule(p, std::move(ctx));
}

hpx::shared_future<void> runtime::when_ready(const object& obj)
{
    return scheduler_->when_ready(obj);
}

namespace
{
std::unique_ptr<runtime> kubus_runtime = {};
std::once_flag kubus_runtime_init_flag;
}

void init()
{
    std::call_once(kubus_runtime_init_flag, []
                   {
                       kubus_runtime = util::make_unique<runtime>();
                   });
}

runtime& get_runtime()
{
    // FIXME: Is this thread-safe ?
    if (!kubus_runtime)
    {
        throw 0;
    }

    return *kubus_runtime;
}
}
}
