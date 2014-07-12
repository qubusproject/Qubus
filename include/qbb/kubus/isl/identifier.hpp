#ifndef QBB_KUBUS_ISL_IDENTIFIER_HPP
#define QBB_KUBUS_ISL_IDENTIFIER_HPP

#include <isl/id.h>

namespace qbb
{
namespace kubus
{
namespace isl
{

class identifier
{
public:
    explicit identifier(isl_id* handle_);

    identifier(const identifier& other);

    ~identifier();

    isl_id* native_handle() const;

    isl_id* release() noexcept;

private:
    isl_id* handle_;
};

}
}
}

#endif