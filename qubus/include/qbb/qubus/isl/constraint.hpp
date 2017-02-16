#ifndef QUBUS_ISL_CONSTRAINT_HPP
#define QUBUS_ISL_CONSTRAINT_HPP

#include <qbb/qubus/isl/local_space.hpp>
#include <qbb/qubus/isl/affine_expr.hpp>

#include <isl/constraint.h>

namespace qubus
{
namespace isl
{

class constraint
{
public:
    explicit constraint(isl_constraint* handle_);

    constraint(const constraint& other);

    ~constraint();

    isl_constraint* native_handle() const;

    isl_constraint* release() noexcept;

    constraint& set_constant(int value);

    constraint& set_coefficient(isl_dim_type type, int pos, int value);

    static constraint equality(local_space ls);

    static constraint inequality(local_space ls);

    static constraint equality(affine_expr aff_expr);

    static constraint inequality(affine_expr aff_expr);

private:
    isl_constraint* handle_;
};

}
}

#endif