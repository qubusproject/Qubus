#ifndef QBB_QUBUS_CPU_PLAN_REGISTRY_HPP
#define QBB_QUBUS_CPU_PLAN_REGISTRY_HPP

#include <qbb/qubus/plan.hpp>

#include <qbb/util/handle.hpp>

#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>

namespace qbb
{
namespace qubus
{

class cpu_runtime;

class cpu_plan
{
public:
    explicit cpu_plan(std::function<void(void* const*, void*)> entry_);

    cpu_plan(const cpu_plan&) = delete;
    cpu_plan& operator=(const cpu_plan&) = delete;

    void execute(const std::vector<void*>& args, cpu_runtime& runtime) const;

private:
    std::function<void(void* const*, void*)> entry_;
};

class cpu_plan_registry
{
public:
    plan register_plan(std::unique_ptr<cpu_plan> p, std::vector<intent> intents);

    const cpu_plan& lookup_plan(const plan& handle) const;

private:
    util::handle_factory handle_fac_;
    std::unordered_map<util::handle, std::unique_ptr<cpu_plan>> plans_;
};
}
}

#endif
