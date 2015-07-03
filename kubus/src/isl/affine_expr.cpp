#include <qbb/kubus/isl/affine_expr.hpp>

namespace qbb
{
namespace qubus
{
namespace isl
{

affine_expr::affine_expr(isl_aff* handle_) : handle_{handle_}
{
}

affine_expr::affine_expr(local_space domain, value val)
: handle_{isl_aff_val_on_domain(domain.release(), val.release())}
{
}

affine_expr::affine_expr(local_space domain, isl_dim_type type, unsigned int pos)
: handle_{isl_aff_var_on_domain(domain.release(), type, pos)}
{
}

affine_expr::affine_expr(const affine_expr& other) : handle_{isl_aff_copy(other.native_handle())}
{
}

affine_expr::~affine_expr()
{
    isl_aff_free(handle_);
}

isl_aff* affine_expr::native_handle() const
{
    return handle_;
}

isl_aff* affine_expr::release() noexcept
{
    isl_aff* temp = handle_;

    handle_ = nullptr;

    return temp;
}

affine_expr operator+(affine_expr lhs, affine_expr rhs)
{
    return affine_expr(isl_aff_add(lhs.release(), rhs.release()));
}

affine_expr operator-(affine_expr lhs, affine_expr rhs)
{
    return affine_expr(isl_aff_sub(lhs.release(), rhs.release()));
}

affine_expr operator*(affine_expr lhs, affine_expr rhs)
{
    return affine_expr(isl_aff_mul(lhs.release(), rhs.release()));
}

affine_expr operator/(affine_expr lhs, affine_expr rhs)
{
    return affine_expr(isl_aff_div(lhs.release(), rhs.release()));
}

affine_expr operator-(affine_expr arg)
{
    return affine_expr(isl_aff_neg(arg.release()));
}

affine_expr floor(affine_expr arg)
{
    return affine_expr(isl_aff_floor(arg.release()));
}

}
}
}