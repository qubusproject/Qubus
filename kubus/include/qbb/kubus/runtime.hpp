#ifndef QBB_KUBUS_RUNTIME_HPP
#define QBB_KUBUS_RUNTIME_HPP

#include <hpx/config.hpp>

#include <qbb/kubus/scheduler.hpp>

#include <qbb/kubus/abi_info.hpp>

#include <qbb/kubus/plan.hpp>
#include <qbb/kubus/execution_context.hpp>
#include <qbb/kubus/IR/function_declaration.hpp>

#include <qbb/kubus/user_defined_plan.hpp>

#include <qbb/kubus/backend_registry.hpp>
#include <qbb/kubus/object_factory.hpp>

#include <memory>
#include <utility>

namespace qbb
{
namespace kubus
{

class global_plan_repository;
    
class runtime_executor : public executor
{
public:
    explicit runtime_executor(global_plan_repository& plan_repository_);
    virtual ~runtime_executor() = default;
    
    hpx::lcos::future<void> execute_plan(const plan& executed_plan, execution_context ctx) override;
private:
    global_plan_repository* plan_repository_;
};

class runtime
{
public: 
    runtime();
    ~runtime();
    
    object_factory& get_object_factory();
    
    plan compile(function_declaration decl);
    plan register_user_defined_plan(user_defined_plan_t plan);
    
    void execute(plan p, execution_context ctx);
    hpx::shared_future<void> when_ready(const object& obj);
private:
    abi_info abi_info_;
    
    backend* cpu_backend_;
    
    object_factory object_factory_;
    
    backend_registry backend_registry_;
    
    std::unique_ptr<global_plan_repository> plan_repository_;
    
    std::unique_ptr<runtime_executor> runtime_exec_;
    std::unique_ptr<scheduler> scheduler_;
};

void init(int argc, char** argv);
runtime& get_runtime();

template<typename... Args>
void execute(const plan& p, Args&&... args)
{
    execution_context ctx({std::forward<Args>(args).get_object()...});
    
    get_runtime().execute(p, std::move(ctx));
}

}
}


#endif