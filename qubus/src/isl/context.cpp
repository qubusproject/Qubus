#include <qbb/qubus/isl/context.hpp>

#include <isl/options.h>

namespace qubus
{
namespace isl
{

context::context() : handle_{isl_ctx_alloc()}
{
    isl_options_set_on_error(handle_, ISL_ON_ERROR_ABORT);
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