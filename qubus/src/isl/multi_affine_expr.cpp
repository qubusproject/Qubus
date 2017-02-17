#include <qubus/isl/multi_affine_expr.hpp>

namespace qubus
{
namespace isl
{
    multi_affine_expr::multi_affine_expr(space s, std::vector<affine_expr> list)
    {
        auto aff_list = isl_aff_list_alloc(s.get_ctx().native_handle(), 0);

        for (auto& expr : list)
        {
            aff_list = isl_aff_list_add(aff_list, expr.release());
        }

        handle_ = isl_multi_aff_from_aff_list(s.release(), aff_list);
    }

    multi_affine_expr::multi_affine_expr(const multi_affine_expr& other)
    : handle_(isl_multi_aff_copy(other.native_handle()))
    {
    }

    multi_affine_expr::~multi_affine_expr()
    {
        isl_multi_aff_free(handle_);
    }

    multi_affine_expr& multi_affine_expr::operator=(multi_affine_expr other)
    {
        isl_multi_aff_free(handle_);

        handle_ = other.release();

        return *this;
    }

    isl_multi_aff* multi_affine_expr::native_handle() const
    {
        return handle_;
    }

    isl_multi_aff* multi_affine_expr::release() noexcept
    {
        isl_multi_aff* temp = handle_;

        handle_ = nullptr;

        return temp;
    }


}
}