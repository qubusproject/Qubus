#ifndef QBB_KUBUS_ISL_MULTI_UNION_PW_AFFINE_EXPR_HPP
#define QBB_KUBUS_ISL_MULTI_UNION_PW_AFFINE_EXPR_HPP

#include <qbb/kubus/isl/map.hpp>

#include <isl/aff.h>

namespace qbb
{
namespace kubus
{
namespace isl
{

class multi_union_pw_affine_expr
{
public:
    explicit multi_union_pw_affine_expr(isl_multi_union_pw_aff* handle_);
    
    multi_union_pw_affine_expr(const multi_union_pw_affine_expr& other);

    ~multi_union_pw_affine_expr();

    isl_multi_union_pw_aff* native_handle() const;

    isl_multi_union_pw_aff* release() noexcept;
    
    static multi_union_pw_affine_expr from_union_map(union_map map);
private:
    isl_multi_union_pw_aff* handle_;
};

}
}
}

#endif