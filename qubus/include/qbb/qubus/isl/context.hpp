#ifndef QBB_QUBUS_ISL_CONTEXT_HPP
#define QBB_QUBUS_ISL_CONTEXT_HPP

#include <isl/ctx.h>

namespace qubus
{
namespace isl
{

class context
{
public:
    context();

    ~context();

    context(const context&) = delete;

    context& operator=(const context&) = delete;

    isl_ctx* native_handle() const;

private:
    isl_ctx* handle_;
};

class context_ref
{
public:
    explicit context_ref(isl_ctx* handle_);
    context_ref(context& ctx);

    isl_ctx* native_handle() const;
private:
    isl_ctx* handle_;
};

}
}

#endif