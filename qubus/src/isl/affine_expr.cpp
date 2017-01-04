#include <qbb/qubus/isl/affine_expr.hpp>

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

affine_expr& affine_expr::operator=(affine_expr other)
{
    isl_aff_free(handle_);

    handle_ = other.release();

    return *this;
}

value affine_expr::constant_value() const
{
    return value(isl_aff_get_constant_val(handle_));
}

space affine_expr::get_space() const
{
    return space(isl_aff_get_space(handle_));
}

context_ref affine_expr::ctx() const
{
    return context_ref(isl_aff_get_ctx(handle_));
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

affine_expr operator+(affine_expr lhs, int rhs)
{
    return affine_expr(isl_aff_add_constant_si(lhs.release(), rhs));
}

affine_expr operator+(int lhs, affine_expr rhs)
{
    return affine_expr(isl_aff_add_constant_si(rhs.release(), lhs));
}

affine_expr operator-(affine_expr lhs, affine_expr rhs)
{
    return affine_expr(isl_aff_sub(lhs.release(), rhs.release()));
}

affine_expr operator-(affine_expr lhs, int rhs)
{
    return lhs + -rhs;
}

affine_expr operator-(int lhs, affine_expr rhs)
{
    return lhs + -rhs;
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

affine_expr operator%(affine_expr lhs, value rhs)
{
    return affine_expr(isl_aff_mod_val(lhs.release(), rhs.release()));
}

affine_expr operator%(affine_expr lhs, long rhs)
{
    return std::move(lhs) % value(lhs.ctx(), rhs);
}

affine_expr floor(affine_expr arg)
{
    return affine_expr(isl_aff_floor(arg.release()));
}

affine_expr align_params(affine_expr aff, space model)
{
    return affine_expr(isl_aff_align_params(aff.release(), model.release()));
}

bool is_cst(const affine_expr& aff)
{
    return isl_aff_is_cst(aff.native_handle());
}

}
}
}