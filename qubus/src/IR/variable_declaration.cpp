#include <qubus/IR/variable_declaration.hpp>

#include <qubus/util/assert.hpp>

#include <utility>

namespace qubus
{

variable_declaration::variable_declaration(std::string name_, type var_type_)
: name_(std::move(name_)), var_type_(std::move(var_type_))
{
}

std::string_view variable_declaration::name() const
{
    return name_;
}

const type& variable_declaration::var_type() const
{
    return var_type_;
}

annotation_map& variable_declaration::annotations() const
{
    return annotations_;
}

annotation_map& variable_declaration::annotations()
{
    return annotations_;
}

}