#include <qubus/IR/variable_declaration.hpp>

#include <qubus/util/assert.hpp>

#include <utility>

namespace qubus
{

variable_declaration_info::variable_declaration_info(std::string name_, type var_type_)
: name_(std::move(name_)), var_type_(std::move(var_type_))
{
    QUBUS_ASSERT(static_cast<bool>(this->var_type_), "Invalid variable type.");
}

const std::string& variable_declaration_info::name() const
{
    return name_;
}

const type& variable_declaration_info::var_type() const
{
    return var_type_;
}

annotation_map& variable_declaration_info::annotations() const
{
    return annotations_;
}

annotation_map& variable_declaration_info::annotations()
{
    return annotations_;
}

variable_declaration::variable_declaration(std::string name_, type var_type_)
: info_(std::make_shared<variable_declaration_info>(std::move(name_), std::move(var_type_)))
{
}

const std::string& variable_declaration::name() const
{
    return info_->name();
}

const type& variable_declaration::var_type() const
{
    return info_->var_type();
}

util::handle variable_declaration::id() const
{
    return util::handle_from_ptr(info_.get());
}

annotation_map& variable_declaration::annotations() const
{
    return info_->annotations();
}
    
annotation_map& variable_declaration::annotations()
{
    return info_->annotations();
}

bool operator==(const variable_declaration& lhs, const variable_declaration& rhs)
{
    return lhs.id() == rhs.id();
}

bool operator!=(const variable_declaration& lhs, const variable_declaration& rhs)
{
    return !(lhs == rhs);
}

}