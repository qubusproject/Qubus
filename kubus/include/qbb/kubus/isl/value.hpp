#ifndef QBB_KUBUS_ISL_VALUE_HPP
#define QBB_KUBUS_ISL_VALUE_HPP

#include <isl/val.h>

namespace qbb
{
namespace kubus
{
namespace isl
{

class context;
    
class value
{
public:
    explicit value(isl_val* handle_);

    value(const context& ctx, long int val);

    value(const value& other);

    ~value();
    
    long int as_integer() const;

    bool is_int() const;
    
    isl_val* native_handle() const;

    isl_val* release() noexcept;

private:
    isl_val* handle_;
};

}
}
}

#endif
