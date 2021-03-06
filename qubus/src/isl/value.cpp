#include <qubus/isl/value.hpp>

#include <qubus/isl/context.hpp>

#include <qubus/util/assert.hpp>

namespace qubus
{
namespace isl
{

value::value(isl_val* handle_) : handle_(handle_)
{
}

value::value(context_ref ctx, long int val)
: handle_(isl_val_int_from_si(ctx.native_handle(), val))
{
}

value::value(const value& other) : handle_(isl_val_copy(other.native_handle()))
{
}

value::~value()
{
    isl_val_free(handle_);
}

long int value::as_integer() const
{
    QUBUS_ASSERT(is_int(), "Value should contain an integer.");
    
    return isl_val_get_num_si(native_handle());
}

bool value::is_int() const
{
    return isl_val_is_int(native_handle()) != 0;
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