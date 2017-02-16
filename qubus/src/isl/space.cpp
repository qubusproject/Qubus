#include <qubus/isl/space.hpp>

#include <qubus/isl/context.hpp>

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

space& space::operator=(space other)
{
    isl_space_free(handle_);

    handle_ = other.release();

    return *this;
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

context_ref space::get_ctx() const
{
    return context_ref(isl_space_get_ctx(handle_));
}

space space::from_domain_and_range(space domain, space range)
{
    return space(isl_space_map_from_domain_and_range(domain.release(), range.release()));
}

space add_dims(space s, isl_dim_type type, unsigned int n)
{
    return space(isl_space_add_dims(s.release(), type, n));
}

space drop_all_dims(space s, isl_dim_type type)
{
    auto n = s.dim(type);
    
    return space(isl_space_drop_dims(s.release(), type, 0, n));
}

space align_params(space s, space model)
{
    return space(isl_space_align_params(s.release(), model.release()));
}
}
}