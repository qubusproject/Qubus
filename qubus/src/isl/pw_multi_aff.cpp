#include <qbb/qubus/isl/pw_multi_aff.hpp>

namespace qbb
{
namespace qubus
{
namespace isl
{

pw_multi_aff::pw_multi_aff(isl_pw_multi_aff* handle_) : handle_{handle_}
{
}

pw_multi_aff::pw_multi_aff(const pw_multi_aff& other) : handle_{isl_pw_multi_aff_copy(other.native_handle())}
{
}

pw_multi_aff::~pw_multi_aff()
{
    isl_pw_multi_aff_free(handle_);
}

pw_multi_aff& pw_multi_aff::operator=(const pw_multi_aff& other)
{
    isl_pw_multi_aff_free(handle_);
    
    handle_ = isl_pw_multi_aff_copy(other.native_handle());
    
    return *this;
}

pw_aff pw_multi_aff::operator[](int pos) const
{
    return pw_aff(isl_pw_multi_aff_get_pw_aff(handle_, pos));
}

isl_pw_multi_aff* pw_multi_aff::native_handle() const
{
    return handle_;
}

isl_pw_multi_aff* pw_multi_aff::release() noexcept
{
    isl_pw_multi_aff* temp = handle_;

    handle_ = nullptr;

    return temp;
}

pw_multi_aff pw_multi_aff::from_map(map m)
{
    return pw_multi_aff(isl_pw_multi_aff_from_map(m.release()));
}

pw_aff pullback(pw_aff lhs, pw_multi_aff rhs)
{
    return pw_aff(isl_pw_aff_pullback_pw_multi_aff(lhs.release(), rhs.release()));
}

pw_multi_aff lexmin_pw_multi_aff(set s)
{
    return pw_multi_aff(isl_set_lexmin_pw_multi_aff(s.release()));
}

pw_multi_aff lexmax_pw_multi_aff(set s)
{
    return pw_multi_aff(isl_set_lexmax_pw_multi_aff(s.release()));
}

}
}
}