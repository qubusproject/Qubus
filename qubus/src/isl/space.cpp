#include <qbb/qubus/isl/space.hpp>

#include <qbb/qubus/isl/context.hpp>

namespace qbb
{
namespace qubus
{
namespace isl
{

space::space(isl_space* handle_) : handle_{handle_}
{
}

space::space(context_ref ctx, unsigned int nparam)
: handle_{isl_space_params_alloc(ctx.native_handle(), nparam)}
{   
}

space::space(context_ref ctx, unsigned int nparam, unsigned int n)
: handle_{isl_space_set_alloc(ctx.native_handle(), nparam, n)}
{
}

space::space(context_ref ctx, unsigned int nparam, unsigned int n_in, unsigned int n_out)
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

unsigned space::dim(isl_dim_type type) const
{
    return isl_space_dim(handle_, type);
}

void space::set_tuple_name(isl_dim_type type, const std::string& name)
{
    handle_ = isl_space_set_tuple_name(handle_, type, name.c_str());
}

void space::set_dim_name(isl_dim_type type, int pos, const std::string& name)
{
    handle_ = isl_space_set_dim_name(handle_, type, pos, name.c_str());
}

boost::string_ref space::get_dim_name(isl_dim_type type, int pos) const
{
    return boost::string_ref(isl_space_get_dim_name(handle_, type, pos));
}

int space::find_dim_by_name(isl_dim_type type, const std::string& name) const
{
    return isl_space_find_dim_by_name(handle_, type, name.c_str());
}

space drop_all_dims(space s, isl_dim_type type)
{
    auto n = s.dim(type);
    
    return space(isl_space_drop_dims(s.release(), type, 0, n));
}
}
}
}