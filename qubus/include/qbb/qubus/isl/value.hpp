#ifndef QBB_QUBUS_ISL_VALUE_HPP
#define QBB_QUBUS_ISL_VALUE_HPP

#include <qbb/qubus/isl/context.hpp>

#include <isl/val.h>

inline namespace qbb
{
namespace qubus
{
namespace isl
{

class context;
    
class value
{
public:
    explicit value(isl_val* handle_);

    value(context_ref ctx, long int val);

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
