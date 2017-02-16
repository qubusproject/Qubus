#include <qubus/jit/load_store.hpp>

#include <llvm/IR/Metadata.h>

#include <hpx/lcos/local/promise.hpp>
#include <hpx/lcos/future.hpp>
#include <hpx/lcos/wait_all.hpp>

#include <utility>

namespace qubus
{
namespace jit
{
llvm::LoadInst* load_from_ref(const reference& ref, llvm_environment& env, compilation_context& ctx)
{
    auto& builder = env.builder();

    auto value = builder.CreateLoad(ref.addr());

    llvm::MDNode* alias_scopes = llvm::MDNode::get(env.ctx(), {});
    llvm::MDNode* noalias_set = llvm::MDNode::get(env.ctx(), {});
    value->setMetadata("alias.scope", alias_scopes);
    value->setMetadata("noalias", noalias_set);

    auto alias_info = ctx.query_global_alias_info(ref);
    ref.add_alias_info(alias_info);

    for (const auto& entry : ref.alias_scopes())
    {
        auto f =
                entry.then(hpx::launch::sync_policies,
                           [value, &env](const hpx::lcos::shared_future<llvm::MDNode*>& alias_scope)
                           {
                               auto alias_scopes = value->getMetadata("alias.scope");

                               std::vector<llvm::Metadata*> scopes = {alias_scope.get()};
                               auto result = llvm::MDNode::concatenate(
                                       alias_scopes, llvm::MDNode::get(env.ctx(), scopes));

                               value->setMetadata("alias.scope", result);
                           });

        ctx.register_pending_task(std::move(f));
    }

    for (const auto& entry : ref.noalias_sets())
    {
        auto f = entry.then(hpx::launch::sync_policies,
                            [value](const hpx::lcos::shared_future<llvm::MDNode*>& noalias_set)
                            {
                                auto noalias_sets = value->getMetadata("noalias");

                                auto result =
                                        llvm::MDNode::concatenate(noalias_sets, noalias_set.get());

                                value->setMetadata("noalias", result);
                            });

        ctx.register_pending_task(std::move(f));
    }

    return value;
}

llvm::StoreInst* store_to_ref(const reference& ref, llvm::Value* value, llvm_environment& env,
                              compilation_context& ctx)
{
    auto& builder = env.builder();

    auto store = builder.CreateStore(value, ref.addr());

    llvm::MDNode* alias_scopes = llvm::MDNode::get(env.ctx(), {});
    llvm::MDNode* noalias_set = llvm::MDNode::get(env.ctx(), {});
    store->setMetadata("alias.scope", alias_scopes);
    store->setMetadata("noalias", noalias_set);

    auto alias_info = ctx.query_global_alias_info(ref);
    ref.add_alias_info(alias_info);

    for (const auto& entry : ref.alias_scopes())
    {
        auto f =
                entry.then([store, &env](const hpx::lcos::shared_future<llvm::MDNode*>& alias_scope)
                           {
                               auto alias_scopes = store->getMetadata("alias.scope");

                               std::vector<llvm::Metadata*> scopes = {alias_scope.get()};
                               auto result = llvm::MDNode::concatenate(
                                       alias_scopes, llvm::MDNode::get(env.ctx(), scopes));

                               store->setMetadata("alias.scope", result);
                           });

        ctx.register_pending_task(std::move(f));
    }

    for (const auto& entry : ref.noalias_sets())
    {
        auto f = entry.then([store](const hpx::lcos::shared_future<llvm::MDNode*>& noalias_set)
                            {
                                auto noalias_sets = store->getMetadata("noalias");

                                auto result =
                                        llvm::MDNode::concatenate(noalias_sets, noalias_set.get());

                                store->setMetadata("noalias", result);
                            });

        ctx.register_pending_task(std::move(f));
    }

    return store;
}

}
}


