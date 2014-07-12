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
}
}
}