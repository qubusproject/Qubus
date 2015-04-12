#ifndef QBB_KUBUS_ISL_PW_MULTI_AFF_HPP
#define QBB_KUBUS_ISL_PW_MULTI_AFF_HPP

#include <qbb/kubus/isl/pw_aff.hpp>
#include <qbb/kubus/isl/set.hpp>

#include <isl/aff.h>

namespace qbb
{
namespace kubus
{
namespace isl
{

class pw_multi_aff
{
public:
    explicit pw_multi_aff(isl_pw_multi_aff* handle_);

    pw_multi_aff(const pw_multi_aff& other);

    ~pw_multi_aff();
    
    pw_multi_aff& operator=(const pw_multi_aff& other);

    pw_aff operator[](int pos) const;
    
    isl_pw_multi_aff* native_handle() const;

    isl_pw_multi_aff* release() noexcept;

private:
    isl_pw_multi_aff* handle_;
};

pw_multi_aff lexmin_pw_multi_aff(set s);
pw_multi_aff lexmax_pw_multi_aff(set s);

}
}
}

#endif