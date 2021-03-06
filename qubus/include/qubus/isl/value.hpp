#ifndef QUBUS_ISL_VALUE_HPP
#define QUBUS_ISL_VALUE_HPP

#include <qubus/isl/context.hpp>

#include <isl/val.h>

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

#endif
