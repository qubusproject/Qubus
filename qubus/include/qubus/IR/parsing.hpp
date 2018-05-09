#ifndef QUBUS_IR_PARSING_HPP
#define QUBUS_IR_PARSING_HPP

#include <qubus/exception.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace qubus
{

class parsing_error : public virtual exception, public virtual std::runtime_error
{
public:
    explicit parsing_error(std::string reason_)
    : std::runtime_error("Error while parsing QIR code: " + std::move(reason_))
    {
    }
};

class module;

std::unique_ptr<module> parse_qir(const std::string& code);
}

#endif
