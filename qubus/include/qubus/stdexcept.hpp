#ifndef QUBUS_STDEXCEPT_HPP
#define QUBUS_STDEXCEPT_HPP

#include <qubus/exception.hpp>

#include <exception>
#include <stdexcept>

namespace qubus
{
    class runtime_error : public virtual exception, public virtual std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    class logic_error : public virtual exception, public virtual std::logic_error
    {
    public:
        using std::logic_error::logic_error;
    };
}

#endif
