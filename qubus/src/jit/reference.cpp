#include <qbb/qubus/jit/reference.hpp>

namespace qubus
{
namespace jit
{

reference::reference(llvm::Value* addr_, access_path origin_, type datatype_)
: addr_(std::move(addr_)), origin_(std::move(origin_)), datatype_(std::move(datatype_))
{
}

llvm::Value* reference::addr() const
{
    return addr_;
}

const access_path& reference::origin() const
{
    return origin_;
}

const type& reference::datatype() const
{
    return datatype_;
}

const std::vector<hpx::lcos::shared_future<llvm::MDNode*>>& reference::alias_scopes() const
{
    return alias_scopes_;
}

const std::vector<hpx::lcos::shared_future<llvm::MDNode*>>& reference::noalias_sets() const
{
    return noalias_sets_;
}

void reference::add_alias_info(const alias_info& info) const
{
    alias_scopes_.push_back(info.alias_scope);
    noalias_sets_.push_back(info.noalias_set);
}
}
}