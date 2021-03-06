#ifndef QUBUS_JIT_COMPILATION_CONTEXT_HPP
#define QUBUS_JIT_COMPILATION_CONTEXT_HPP

#include <hpx/config.hpp>

#include <qubus/IR/function.hpp>

#include <qubus/jit/alias_info.hpp>
#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/reference.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/lcos/local/promise.hpp>
#include <hpx/lcos/wait_all.hpp>

#include <boost/optional.hpp>
#include <boost/signals2.hpp>

#include <qubus/util/handle.hpp>
#include <qubus/util/optional_ref.hpp>

#include <memory>
#include <map>

namespace qubus
{
namespace jit
{

class scope
{
public:
    using on_scope_exit_signal = boost::signals2::signal<void()>;

    scope();

    scope(const scope&) = delete;
    scope& operator=(const scope&) = delete;

    scope(scope&&) = default;
    scope& operator=(scope&&) = default;

    ~scope();

    void on_exit(const on_scope_exit_signal::slot_type& subscriber);

private:
    std::unique_ptr<on_scope_exit_signal> on_scope_exit_;
};

class global_alias_info_query
{
public:
    explicit global_alias_info_query(reference ref_);

    global_alias_info_query(const global_alias_info_query&) = delete;
    global_alias_info_query& operator=(const global_alias_info_query&) = delete;

    global_alias_info_query(global_alias_info_query&&) = default;
    global_alias_info_query& operator=(global_alias_info_query&&) = default;

    const reference& ref() const;

    void answer(llvm::MDNode* alias_scope, llvm::MDNode* alias_set);

    hpx::lcos::shared_future<llvm::MDNode*> get_alias_scope();
    hpx::lcos::shared_future<llvm::MDNode*> get_alias_set();

private:
    reference ref_;
    hpx::lcos::local::promise<llvm::MDNode*> alias_scope_promise_;
    hpx::lcos::local::promise<llvm::MDNode*> alias_set_promise_;
};

class compilation_context
{
public:
    explicit compilation_context(llvm_environment& env_);
    ~compilation_context();

    compilation_context(const compilation_context&) = delete;
    compilation_context& operator=(const compilation_context&) = delete;

    std::map<util::handle, reference>& symbol_table();

    const std::map<util::handle, reference>& symbol_table() const;

    scope& enter_new_scope();

    scope& get_current_scope();

    const scope& get_current_scope() const;

    void exit_current_scope();

    alias_info query_global_alias_info(const reference& ref) const;

    void register_pending_task(hpx::lcos::future<void> f);

    void wait_on_pending_tasks() const;

private:
    void answer_pending_global_alias_queries();

    llvm_environment* env_;

    std::map<util::handle, reference> symbol_table_;
    std::vector<scope> scopes_;

    mutable std::vector<global_alias_info_query> pending_global_alias_queries_;

    std::vector<hpx::lcos::future<void>> pending_tasks_;
};
}
}

#endif