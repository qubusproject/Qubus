#include <hpx/config.hpp>

#include <qbb/qubus/local_runtime.hpp>

#include <qbb/qubus/plan.hpp>

#include <qbb/qubus/lower_top_level_sums.hpp>
#include <qbb/qubus/sparse_patterns.hpp>
#include <qbb/qubus/lower_abstract_indices.hpp>
#include <qbb/qubus/loop_optimizer.hpp>
#include <qbb/qubus/make_implicit_conversions_explicit.hpp>
#include <qbb/qubus/IR/pretty_printer.hpp>
#include <qbb/qubus/multi_index_handling.hpp>
#include <qbb/qubus/kronecker_delta_folding_pass.hpp>

#include <qbb/qubus/metadata_builder.hpp>
#include <qbb/qubus/jit/execution_stack.hpp>

#include <qbb/qubus/logging.hpp>

#include <boost/range/adaptor/indexed.hpp>

#include <qbb/util/get_prefix.hpp>
#include <qbb/util/make_unique.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/include/lcos.hpp>

#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <iterator>

namespace qbb
{
namespace qubus
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

}

// TODO: Guard this with a mutex
class global_plan_repository
{
public:
    plan add_plan(backend* backend, plan handle)
    {
        auto id = id_factory_.create();

        plans_.emplace(id, util::make_unique<simple_plan>(backend, handle));

        return plan(id, handle.intents());
    }

    runtime_plan* lookup_plan(const plan& p) const
    {
        return plans_.at(p.id()).get();
    }

private:
    util::handle_factory id_factory_;
    std::unordered_map<util::handle, std::unique_ptr<runtime_plan>> plans_;
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

local_runtime::local_runtime()
: cpu_plugin_(util::get_prefix("qubus") / "qubus/backends/libqubus_cpu_backend.so"),
  object_factory_(abi_info_)
{
    init_logging();

    BOOST_LOG_NAMED_SCOPE("runtime");

    logger slg;

    QUBUS_LOG(slg, normal) << "Initialize the Qubus runtime";

    QUBUS_LOG(slg, normal) << "Runtime prefix: " << util::get_prefix("qubus");

    QUBUS_LOG(slg, normal) << "Bootstrapping virtual multiprocessor";

    QUBUS_LOG(slg, normal) << "Scanning for backends";

    QUBUS_LOG(slg, normal) << "Loading backend 'cpu_backend'";

    auto init_cpu_backend = cpu_plugin_.get<backend*(const abi_info*)>("init_cpu_backend");

    cpu_backend_ = dynamic_cast<host_backend*>(init_cpu_backend(&abi_info_));

    if (!cpu_backend_)
    {
        throw 0;
    }

    plan_repository_ =
        util::make_unique<global_plan_repository>();

    runtime_exec_ = util::make_unique<runtime_executor>(*plan_repository_);

    scheduler_ = util::make_unique<greedy_scheduler>(*runtime_exec_);
}

local_runtime::~local_runtime()
{
}

object_factory& local_runtime::get_object_factory()
{
    return object_factory_;
}

plan local_runtime::compile(function_declaration decl)
{
    decl = expand_multi_indices(decl);

    decl = fold_kronecker_deltas(decl);

    decl = optimize_sparse_patterns(decl);

    decl = lower_top_level_sums(decl);

    decl = lower_abstract_indices(decl);

    decl = optimize_loops(decl);

    decl = make_implicit_conversions_explicit(decl);

    auto& compiler = cpu_backend_->get_compiler();

    return plan_repository_->add_plan(cpu_backend_, compiler.compile_plan(decl));
}

plan local_runtime::register_user_defined_plan(user_defined_plan_t plan)
{
    auto cpu_plan = cpu_backend_->register_function_as_plan(plan.body, plan.intents);

    return plan_repository_->add_plan(cpu_backend_, cpu_plan);
}

void local_runtime::execute(plan p, execution_context ctx)
{
    scheduler_->schedule(p, std::move(ctx));
}

hpx::shared_future<void> local_runtime::when_ready(const object& obj)
{
    return scheduler_->when_ready(obj);
}

namespace
{
std::unique_ptr<local_runtime> qubus_runtime = {};
std::once_flag qubus_runtime_init_flag;
}

void init(int QBB_UNUSED(argc), char** QBB_UNUSED(argv))
{
    std::call_once(qubus_runtime_init_flag, []
                   {
                       qubus_runtime = util::make_unique<local_runtime>();
                   });
}

local_runtime& get_runtime()
{
    // FIXME: Is this thread-safe ?
    if (!qubus_runtime)
    {
        throw 0;
    }

    return *qubus_runtime;
}
}
}
