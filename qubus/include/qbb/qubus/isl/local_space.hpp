#ifndef QBB_QUBUS_ISL_LOCAL_SPACE_HPP
#define QBB_QUBUS_ISL_LOCAL_SPACE_HPP

#include <qbb/qubus/isl/space.hpp>

#include <isl/local_space.h>

namespace qubus
{
namespace isl
{

class context;

class local_space
{
public:
    local_space(space s);

    local_space(const local_space& other);

    ~local_space();

    isl_local_space* native_handle() const;

    isl_local_space* release() noexcept;

private:
    isl_local_space* handle_;
};

}
}

#endif