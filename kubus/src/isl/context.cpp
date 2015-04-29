#include <qbb/kubus/isl/context.hpp>

namespace qbb
{
namespace kubus
{
namespace isl
{

context::context() : handle_{isl_ctx_alloc()}
{
}

context::~context()
{
    isl_ctx_free(handle_);
}

isl_ctx* context::native_handle() const
{
    return handle_;
}

context_ref::context_ref(isl_ctx* handle_) : handle_(handle_)
{
}

context_ref::context_ref(context& ctx) : handle_(ctx.native_handle())
{
}

isl_ctx* context_ref::native_handle() const
{
    return handle_;
}
}
}
}