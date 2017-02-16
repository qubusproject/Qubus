#include <qbb/qubus/isl/multi_union_pw_affine_expr.hpp>

namespace qubus
{
namespace isl
{

multi_union_pw_affine_expr::multi_union_pw_affine_expr(isl_multi_union_pw_aff* handle_)
: handle_(handle_)
{
}

multi_union_pw_affine_expr::multi_union_pw_affine_expr(affine_expr aff)
: handle_(isl_multi_union_pw_aff_from_union_pw_aff(
      isl_union_pw_aff_from_pw_aff(isl_pw_aff_from_aff(aff.release()))))
{
}

multi_union_pw_affine_expr::multi_union_pw_affine_expr(const multi_union_pw_affine_expr& other)
: handle_(isl_multi_union_pw_aff_copy(other.native_handle()))
{
}
multi_union_pw_affine_expr::~multi_union_pw_affine_expr()
{
    isl_multi_union_pw_aff_free(handle_);
}
isl_multi_union_pw_aff* multi_union_pw_affine_expr::native_handle() const
{
    return handle_;
}
isl_multi_union_pw_aff* multi_union_pw_affine_expr::release() noexcept
{
    isl_multi_union_pw_aff* tmp = handle_;

    handle_ = nullptr;

    return tmp;
}
multi_union_pw_affine_expr multi_union_pw_affine_expr::from_union_map(union_map map)
{
    return multi_union_pw_affine_expr(isl_multi_union_pw_aff_from_union_map(map.release()));
}
}
}