#ifndef QBB_QUBUS_ISL_AFFINE_EXPR_HPP
#define QBB_QUBUS_ISL_AFFINE_EXPR_HPP

#include <qbb/kubus/isl/local_space.hpp>
#include <qbb/kubus/isl/value.hpp>

#include <isl/aff.h>

namespace qbb
{
namespace qubus
{
namespace isl
{

class affine_expr
{
public:
    explicit affine_expr(isl_aff* handle_);

    affine_expr(local_space domain, value val);

    affine_expr(local_space domain, isl_dim_type type, unsigned int pos);

    affine_expr(const affine_expr& other);

    ~affine_expr();

    isl_aff* native_handle() const;

    isl_aff* release() noexcept;

private:
    isl_aff* handle_;
};

affine_expr operator+(affine_expr lhs, affine_expr rhs);

affine_expr operator-(affine_expr lhs, affine_expr rhs);

affine_expr operator*(affine_expr lhs, affine_expr rhs);

affine_expr operator/(affine_expr lhs, affine_expr rhs);

affine_expr operator-(affine_expr arg);

affine_expr floor(affine_expr arg);

}
}
}

#endif