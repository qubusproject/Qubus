#ifndef QBB_KUBUS_ISL_PW_AFF_HPP
#define QBB_KUBUS_ISL_PW_AFF_HPP

#include <isl/aff.h>

namespace qbb
{
namespace kubus
{
namespace isl
{

class pw_aff
{
public:
    explicit pw_aff(isl_pw_aff* handle_);

    pw_aff(const pw_aff& other);

    ~pw_aff();
    
    pw_aff& operator=(const pw_aff& other);

    isl_pw_aff* native_handle() const;

    isl_pw_aff* release() noexcept;

private:
    isl_pw_aff* handle_;
};

}
}
}

#endif