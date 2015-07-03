#include <qbb/qubus/isl/constraint.hpp>

namespace qbb
{
namespace qubus
{
namespace isl
{

    constraint::constraint(isl_constraint* handle_) : handle_{handle_}
    {
    }

    constraint::constraint(const constraint& other) : handle_{isl_constraint_copy(other.native_handle())}
    {
    }

    constraint::~constraint()
    {
        isl_constraint_free(handle_);
    }

    isl_constraint* constraint::native_handle() const
    {
        return handle_;
    }

    isl_constraint* constraint::release() noexcept
    {
        isl_constraint* temp = handle_;

        handle_ = nullptr;

        return temp;
    }

    constraint& constraint::set_constant(int value)
    {
        handle_ = isl_constraint_set_constant_si(handle_, value);

        return *this;
    }

    constraint& constraint::set_coefficient(isl_dim_type type, int pos, int value)
    {
        handle_ = isl_constraint_set_coefficient_si(handle_, type, pos, value);

        return *this;
    }

    constraint constraint::equality(local_space ls)
    {
        return constraint(isl_equality_alloc(ls.release()));
    }

    constraint constraint::inequality(local_space ls)
    {
        return constraint(isl_inequality_alloc(ls.release()));
    }

    constraint constraint::equality(affine_expr aff_expr)
    {
        return constraint(isl_equality_from_aff(aff_expr.release()));
    }

    constraint constraint::inequality(affine_expr aff_expr)
    {
        return constraint(isl_inequality_from_aff(aff_expr.release()));
    }

}
}
}