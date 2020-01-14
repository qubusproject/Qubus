#ifndef QUBUS_IR_DEFINITION_HPP
#define QUBUS_IR_DEFINITION_HPP

#include <string_view>

namespace qubus
{

class definition
{
public:
    explicit definition() = default;
    virtual ~definition();

    definition(const definition&) = delete;
    definition& operator=(const definition&) = delete;

    definition(definition&&) = delete;
    definition& operator=(definition&&) = delete;

    virtual std::string_view name() const = 0;
};

}

#endif
