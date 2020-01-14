#ifndef QUBUS_LLVM_ENVIRONMENT_HPP
#define QUBUS_LLVM_ENVIRONMENT_HPP

#include <hpx/config.hpp>

#include <qubus/jit/runtime_library.hpp>

#include <qubus/IR/type.hpp>
#include <qubus/IR/expression.hpp>

#pragma push_macro("DEBUG")
#undef DEBUG

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>

#pragma pop_macro("DEBUG")

#include <qubus/util/handle.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

namespace qubus
{
namespace jit
{
class access_path
{
public:
    access_path() : root_(0), components_{"local"}
    {
    }

    explicit access_path(util::handle root_) : root_(root_)
    {
    }

    std::string str() const
    {
        std::string result = root_.str();

        for (const auto& component : components_)
        {
            result += ".";
            result += component;
        }

        return result;
    }

    access_path& member_access(std::string component)
    {
        components_.push_back(component);

        return *this;
    }

private:
    util::handle root_;
    std::vector<std::string> components_;
};

inline access_path operator/(access_path path, std::string component)
{
    path.member_access(component);

    return path;
}

class compiler;

class llvm_environment
{
public:
    explicit llvm_environment(compiler& compiler_);

    llvm::LLVMContext& ctx() const;

    llvm::IRBuilder<>& builder();

    llvm::MDBuilder& md_builder();

    runtime_library& rt_library();

    llvm::Type* map_qubus_type(const type& t) const;

    llvm::MDNode* get_alias_scope(const access_path& path) const;

    llvm::MDNode* get_noalias_set(const access_path& path) const;

    bool bind_symbol(const std::string& symbol, llvm::Value* value);

    bool unbind_symbol(const std::string& symbol);

    llvm::Value* lookup_symbol(const std::string& symbol) const;

    llvm::Function* get_current_function() const;

    void set_current_function(llvm::Function* func);

    llvm::Function* get_assume_align(llvm::Module& mod);

    llvm::Function* get_alloc_scratch_mem(llvm::Module& mod);

    llvm::Function* get_dealloc_scratch_mem(llvm::Module& mod);
private:
    llvm::LLVMContext* ctx_;

    mutable llvm::IRBuilder<> builder_;
    mutable llvm::MDBuilder md_builder_;

    runtime_library rt_library_;

    llvm::MDNode* global_alias_domain_;

    llvm::Function* current_function_;

    mutable std::map<std::string, llvm::MDNode*> alias_scope_table_;
    std::map<std::string, llvm::Value*> symbol_table_;
};
}
}

#endif