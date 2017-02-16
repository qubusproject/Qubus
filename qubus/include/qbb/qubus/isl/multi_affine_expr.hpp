#ifndef QUBUS_MULTI_AFFINE_EXPR_HPP
#define QUBUS_MULTI_AFFINE_EXPR_HPP

#include <qbb/qubus/isl/affine_expr.hpp>
#include <qbb/qubus/isl/space.hpp>

#include <isl/aff.h>

#include <vector>

namespace qubus
{
namespace isl
{

class multi_affine_expr
{
public:
    multi_affine_expr(space s, std::vector<affine_expr> list);

    multi_affine_expr(const multi_affine_expr& other);

    ~multi_affine_expr();

    multi_affine_expr& operator=(multi_affine_expr other);

    isl_multi_aff* native_handle() const;

    isl_multi_aff* release() noexcept;

private:
    isl_multi_aff* handle_;
};

}
}

#endif
