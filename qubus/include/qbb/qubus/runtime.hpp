#ifndef QBB_QUBUS_RUNTIME_HPP
#define QBB_QUBUS_RUNTIME_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/scheduler.hpp>

#include <qbb/qubus/abi_info.hpp>

#include <qbb/qubus/plan.hpp>
#include <qbb/qubus/execution_context.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>

#include <qbb/qubus/user_defined_plan.hpp>

#include <qbb/qubus/backend_registry.hpp>
#include <qbb/qubus/object_factory.hpp>

#include <boost/dll.hpp>

#include <memory>
#include <utility>

namespace qbb
{
namespace qubus
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
    boost::dll::shared_library cpu_plugin_;
    
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