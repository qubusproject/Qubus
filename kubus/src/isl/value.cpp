#include <qbb/kubus/isl/value.hpp>

#include <qbb/kubus/isl/context.hpp>

namespace qbb
{
namespace kubus
{
namespace isl
{

value::value(isl_val* handle_) : handle_{handle_}
{
}

value::value(const context& ctx, long int val)
: handle_{isl_val_int_from_si(ctx.native_handle(), val)}
{
}

value::value(const value& other) : handle_{isl_val_copy(other.native_handle())}
{
}

value::~value()
{
    isl_val_free(handle_);
}

isl_val* value::native_handle() const
{
    return handle_;
}

isl_val* value::release() noexcept
{
    isl_val* temp = handle_;

    handle_ = nullptr;

    return temp;
}
}
}
}