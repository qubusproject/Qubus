#ifndef QBB_KUBUS_REFERENCE_HPP
#define QBB_KUBUS_REFERENCE_HPP

#include <hpx/config.hpp>

#include <qbb/kubus/backends/alias_info.hpp>

#include <hpx/lcos/future.hpp>

#include <llvm/IR/Value.h>
#include <llvm/IR/Metadata.h>

#include <qbb/kubus/backends/llvm_environment.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class reference
{
public:
    reference() = default;

    reference(llvm::Value* addr_, access_path origin_) : addr_(addr_), origin_(origin_)
    {
    }

    llvm::Value* addr() const
    {
        return addr_;
    }

    const access_path& origin() const
    {
        return origin_;
    }

    const std::vector<hpx::lcos::shared_future<llvm::MDNode*>>& alias_scopes() const
    {
        return alias_scopes_;
    }

    const std::vector<hpx::lcos::shared_future<llvm::MDNode*>>& noalias_sets() const
    {
        return noalias_sets_;
    }

    void add_alias_info(const alias_info& info) const
    {
        alias_scopes_.push_back(info.alias_scope);
        noalias_sets_.push_back(info.noalias_set);
    }
private:
    llvm::Value* addr_;
    access_path origin_;
    mutable std::vector<hpx::lcos::shared_future<llvm::MDNode*>> alias_scopes_;
    mutable std::vector<hpx::lcos::shared_future<llvm::MDNode*>> noalias_sets_;
};

}
}

#endif
