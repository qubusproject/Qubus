#include <qbb/kubus/isl/pw_aff.hpp>

namespace qbb
{
namespace qubus
{
namespace isl
{

pw_aff::pw_aff(isl_pw_aff* handle_) : handle_{handle_}
{
}

pw_aff::pw_aff(const pw_aff& other) : handle_{isl_pw_aff_copy(other.native_handle())}
{
}

pw_aff::~pw_aff()
{
    isl_pw_aff_free(handle_);
}

pw_aff& pw_aff::operator=(const pw_aff& other)
{
    isl_pw_aff_free(handle_);
    
    handle_ = isl_pw_aff_copy(other.native_handle());
    
    return *this;
}

isl_pw_aff* pw_aff::native_handle() const
{
    return handle_;
}

isl_pw_aff* pw_aff::release() noexcept
{
    isl_pw_aff* temp = handle_;

    handle_ = nullptr;

    return temp;
}

set pw_aff::domain() const
{
    return set(isl_pw_aff_domain(isl_pw_aff_copy(handle_)));
}

bool pw_aff::is_cst() const
{
    return isl_pw_aff_is_cst(handle_);
}

pw_aff pw_aff::from_val(set domain, value val)
{
    return pw_aff(isl_pw_aff_val_on_domain(domain.release(), val.release()));
}

pw_aff operator+(pw_aff lhs, pw_aff rhs)
{
    return pw_aff(isl_pw_aff_add(lhs.release(), rhs.release()));
}

pw_aff operator-(pw_aff lhs, pw_aff rhs)
{
    return pw_aff(isl_pw_aff_sub(lhs.release(), rhs.release()));
}

set set_from_pw_aff(pw_aff fn)
{
    return set(isl_set_from_pw_aff(fn.release()));
}

map map_from_pw_aff(pw_aff fn)
{
    return map(isl_map_from_pw_aff(fn.release()));
}

}
}
}