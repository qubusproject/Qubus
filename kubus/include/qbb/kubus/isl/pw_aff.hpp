#ifndef QBB_KUBUS_ISL_PW_AFF_HPP
#define QBB_KUBUS_ISL_PW_AFF_HPP

#include <qbb/kubus/isl/set.hpp>
#include <qbb/kubus/isl/map.hpp>
#include <qbb/kubus/isl/value.hpp>

#include <isl/aff.h>

namespace qbb
{
namespace qubus
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

    set domain() const;

    bool is_cst() const;

    static pw_aff from_val(set domain, value v);
private:
    isl_pw_aff* handle_;
};

pw_aff operator+(pw_aff lhs, pw_aff rhs);
pw_aff operator-(pw_aff lhs, pw_aff rhs);

set set_from_pw_aff(pw_aff fn);
map map_from_pw_aff(pw_aff fn);

}
}
}

#endif