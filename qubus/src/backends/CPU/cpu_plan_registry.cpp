#include <qbb/qubus/backends/cpu_plan_registry.hpp>

namespace qbb
{
namespace qubus
{
cpu_plan::cpu_plan(std::function<void(void* const*, void*)> entry_) : entry_(std::move(entry_))
{
}

void cpu_plan::execute(const std::vector<void*>& args, cpu_runtime& runtime) const
{
    entry_(args.data(), &runtime);
}

plan cpu_plan_registry::register_plan(std::unique_ptr<cpu_plan> p, std::vector<intent> intents)
{
    auto plan_handle = handle_fac_.create();

    plans_.emplace(plan_handle, std::move(p));

    return plan(plan_handle, std::move(intents));
}

const cpu_plan& cpu_plan_registry::lookup_plan(const plan& handle) const
{
    return *plans_.at(handle.id());
}
}
}
