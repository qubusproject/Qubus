#ifndef QBB_QUBUS_ALIAS_INFO_HPP
#define QBB_QUBUS_ALIAS_INFO_HPP

#include <hpx/config.hpp>
#include <hpx/lcos/future.hpp>

#include <llvm/IR/Metadata.h>

#include <utility>

namespace qbb
{
namespace qubus
{
struct alias_info
{
    alias_info(hpx::lcos::shared_future<llvm::MDNode*> alias_scope,
               hpx::lcos::shared_future<llvm::MDNode*> noalias_set)
    : alias_scope(std::move(alias_scope)), noalias_set(std::move(noalias_set))
    {
    }

    hpx::lcos::shared_future<llvm::MDNode*> alias_scope;
    hpx::lcos::shared_future<llvm::MDNode*> noalias_set;
};
}
}

#endif
