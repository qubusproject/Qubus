#ifndef QBB_QUBUS_REFERENCE_HPP
#define QBB_QUBUS_REFERENCE_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/jit/llvm_environment.hpp>
#include <qbb/qubus/jit/alias_info.hpp>

#include <hpx/lcos/future.hpp>

#include <llvm/IR/Value.h>
#include <llvm/IR/Metadata.h>
#include <qbb/qubus/IR/type.hpp>

#include <vector>
#include <utility>

namespace qubus
{
namespace jit
{
class reference
{
public:
    reference() = default;

    reference(llvm::Value* addr_, access_path origin_, type datatype_);

    llvm::Value* addr() const;

    const access_path& origin() const;

    const type& datatype() const;

    const std::vector<hpx::lcos::shared_future<llvm::MDNode*>>& alias_scopes() const;

    const std::vector<hpx::lcos::shared_future<llvm::MDNode*>>& noalias_sets() const;

    void add_alias_info(const alias_info& info) const;

private:
    llvm::Value* addr_;
    access_path origin_;
    type datatype_;
    mutable std::vector<hpx::lcos::shared_future<llvm::MDNode*>> alias_scopes_;
    mutable std::vector<hpx::lcos::shared_future<llvm::MDNode*>> noalias_sets_;
};
}
}

#endif
