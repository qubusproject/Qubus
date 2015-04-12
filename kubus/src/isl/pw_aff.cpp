#include <qbb/kubus/isl/pw_aff.hpp>

namespace qbb
{
namespace kubus
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

}
}
}