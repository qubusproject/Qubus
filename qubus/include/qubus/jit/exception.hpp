#include <qubus/exception.hpp>
#include <stdexcept>

#ifndef QUBUS_JIT_EXCEPTION_HPP
#define QUBUS_JIT_EXCEPTION_HPP

namespace qubus
{
namespace jit
{
class exception : public ::qubus::exception
{
};

class compilation_error : public virtual exception, public virtual std::runtime_error
{
public:
    explicit compilation_error(const std::string& what_) : std::runtime_error(what_)
    {
    }

    explicit compilation_error(const char* what_) : std::runtime_error(what_)
    {
    }
};
} // namespace jit
} // namespace qubus

#endif
