#include <qbb/qubus/isl/local_space.hpp>

#include <qbb/qubus/isl/context.hpp>

namespace qubus
{
namespace isl
{

local_space::local_space(space s) : handle_{isl_local_space_from_space(s.release())}
{
}

local_space::local_space(const local_space& other)
: handle_{isl_local_space_copy(other.native_handle())}
{
}

local_space::~local_space()
{
    isl_local_space_free(handle_);
}

isl_local_space* local_space::native_handle() const
{
    return handle_;
}

isl_local_space* local_space::release() noexcept
{
    isl_local_space* temp = handle_;

    handle_ = nullptr;

    return temp;
}
}
}