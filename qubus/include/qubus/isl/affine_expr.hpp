#ifndef QUBUS_ISL_AFFINE_EXPR_HPP
#define QUBUS_ISL_AFFINE_EXPR_HPP

#include <qubus/isl/local_space.hpp>
#include <qubus/isl/value.hpp>
#include <qubus/isl/context.hpp>

#include <isl/aff.h>

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
    affine_expr(affine_expr&& other) noexcept;

    ~affine_expr();

    affine_expr& operator=(affine_expr other);

    value constant_value() const;

    space get_space() const;

    context_ref ctx() const;

    isl_aff* native_handle() const;

    isl_aff* release() noexcept;

private:
    isl_aff* handle_;
};

affine_expr operator+(affine_expr lhs, affine_expr rhs);
affine_expr operator+(affine_expr lhs, int rhs);
affine_expr operator+(int lhs, affine_expr rhs);

affine_expr operator-(affine_expr lhs, affine_expr rhs);
affine_expr operator-(affine_expr lhs, int rhs);
affine_expr operator-(int lhs, affine_expr rhs);

affine_expr operator*(affine_expr lhs, affine_expr rhs);

affine_expr operator/(affine_expr lhs, affine_expr rhs);

affine_expr operator-(affine_expr arg);

affine_expr operator%(affine_expr lhs, value rhs);
affine_expr operator%(affine_expr lhs, long rhs);

affine_expr floor(affine_expr arg);

affine_expr align_params(affine_expr aff, space model);

bool is_cst(const affine_expr& aff);

}
}

#endif