#include <qbb/kubus/isl/space.hpp>

#include <qbb/kubus/isl/context.hpp>

namespace qbb
{
namespace kubus
{
namespace isl
{

space::space(isl_space* handle_) : handle_{handle_}
{
}

space::space(const context& ctx, unsigned int nparam, unsigned int n)
: handle_{isl_space_set_alloc(ctx.native_handle(), nparam, n)}
{
}

space::space(const context& ctx, unsigned int nparam, unsigned int n_in, unsigned int n_out)
: handle_{isl_space_alloc(ctx.native_handle(), nparam, n_in, n_out)}
{
}

space::space(const space& other) : handle_{isl_space_copy(other.native_handle())}
{
}

space::~space()
{
    isl_space_free(handle_);
}

isl_space* space::native_handle() const
{
    return handle_;
}

isl_space* space::release() noexcept
{
    isl_space* temp = handle_;

    handle_ = nullptr;

    return temp;
}

void space::set_tuple_name(isl_dim_type type, const std::string& name)
{
    handle_ = isl_space_set_tuple_name(handle_, type, name.c_str());
}

void space::set_dim_name(isl_dim_type type, int pos, const std::string& name)
{
    handle_ = isl_space_set_dim_name(handle_, type, pos, name.c_str());
}
}
}
}