#ifndef QBB_HOST_BACKEND_HPP
#define QBB_HOST_BACKEND_HPP

#include <qbb/qubus/backend.hpp>
#include <qbb/qubus/plan.hpp>

#include <functional>
#include <vector>

namespace qbb
{
namespace qubus
{

class host_backend : public backend
{
public:
    host_backend() = default;
    virtual ~host_backend() = default;

    host_backend(const host_backend&) = delete;
    host_backend& operator=(const host_backend&) = delete;

    virtual plan register_function_as_plan(std::function<void(void* const*)> func,
                                           std::vector<intent> intents) = 0;
};
}
}

#endif
