#include <qbb/qubus/jit/compilation_context.hpp>

#include <qbb/util/unused.hpp>

namespace qubus
{
namespace jit
{

scope::scope() : on_scope_exit_(std::make_unique<on_scope_exit_signal>())
{
}

scope::~scope()
{
    if (on_scope_exit_)
    {
        (*on_scope_exit_)();
    }
}

void scope::on_exit(const on_scope_exit_signal::slot_type& subscriber)
{
    on_scope_exit_->connect(subscriber);
}

global_alias_info_query::global_alias_info_query(reference ref_) : ref_(std::move(ref_))
{
}

const reference& global_alias_info_query::ref() const
{
    return ref_;
}

void global_alias_info_query::answer(llvm::MDNode* alias_scope, llvm::MDNode* alias_set)
{
    alias_scope_promise_.set_value(alias_scope);
    alias_set_promise_.set_value(alias_set);
}

hpx::lcos::shared_future<llvm::MDNode*> global_alias_info_query::get_alias_scope()
{
    return alias_scope_promise_.get_future();
}

hpx::lcos::shared_future<llvm::MDNode*> global_alias_info_query::get_alias_set()
{
    return alias_set_promise_.get_future();
}

compilation_context::compilation_context(llvm_environment& env_)
: env_(&env_), scopes_(1)
{
}

compilation_context::~compilation_context()
{
    answer_pending_global_alias_queries();

    wait_on_pending_tasks();
}

std::map<util::handle, reference>& compilation_context::symbol_table()
{
    return symbol_table_;
}

const std::map<util::handle, reference>& compilation_context::symbol_table() const
{
    return symbol_table_;
}

boost::optional<function_declaration> compilation_context::get_next_plan_to_compile()
{
    if (plans_to_compile_.empty())
        return boost::none;

    auto next_plan_to_compile = plans_to_compile_.back();

    plans_to_compile_.pop_back();

    return next_plan_to_compile;
}

void compilation_context::add_plan_to_compile(function_declaration fn)
{
    plans_to_compile_.push_back(std::move(fn));
}

scope& compilation_context::enter_new_scope()
{
    scopes_.emplace_back();

    return scopes_.back();
}

scope& compilation_context::get_current_scope()
{
    return scopes_.back();
}

const scope& compilation_context::get_current_scope() const
{
    return scopes_.back();
}

void compilation_context::exit_current_scope()
{
    scopes_.pop_back();
}

alias_info compilation_context::query_global_alias_info(const reference& ref) const
{
    pending_global_alias_queries_.emplace_back(ref);

    auto& query = pending_global_alias_queries_.back();

    return alias_info(query.get_alias_scope(), query.get_alias_set());
}

void compilation_context::register_pending_task(hpx::lcos::future<void> f)
{
    pending_tasks_.push_back(std::move(f));
}

void compilation_context::wait_on_pending_tasks() const
{
    hpx::wait_all(pending_tasks_);
}

void compilation_context::answer_pending_global_alias_queries()
{
    std::map<std::string, llvm::MDNode*> alias_scope_table;

    llvm::MDNode* global_alias_domain =
        env_->md_builder().createAliasScopeDomain("qubus.alias_domain");

    for (const auto& query : pending_global_alias_queries_)
    {
        std::string name = query.ref().origin().str();

        alias_scope_table[name] = env_->md_builder().createAliasScope(name, global_alias_domain);
    }

    for (auto& query : pending_global_alias_queries_)
    {
        std::string name = query.ref().origin().str();

        std::vector<llvm::Metadata*> alias_scopes;

        for (const auto& entry : alias_scope_table)
        {
            if (entry.first != name)
            {
                alias_scopes.push_back(entry.second);
            }
        }

        auto noalias_set = llvm::MDNode::get(env_->ctx(), alias_scopes);

        auto alias_scope = alias_scope_table.at(name);

        query.answer(alias_scope, noalias_set);
    }
}
}
}
