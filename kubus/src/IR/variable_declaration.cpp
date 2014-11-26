#include <qbb/kubus/IR/variable_declaration.hpp>

namespace qbb
{
namespace kubus
{

class variable_declaration_info
{
public:
    explicit variable_declaration_info(type var_type_, variable_intent intent_)
    : var_type_(var_type_), intent_(intent_)
    {
    }
    
    variable_declaration_info(const variable_declaration_info&) = delete;
    variable_declaration_info& operator=(const variable_declaration_info&) = delete;
    
    const type& var_type() const
    {
        return var_type_;
    }

    variable_intent intent() const
    {
        return intent_;
    }

    annotation_map& annotations() const
    {
        return annotations_;
    }
        
    annotation_map& annotations()
    {
        return annotations_;
    }
private:
    type var_type_;
    variable_intent intent_;
    
    mutable annotation_map annotations_;
};
    
variable_declaration::variable_declaration(type var_type_, variable_intent intent_)
: info_(std::make_shared<variable_declaration_info>(var_type_, intent_))
{
}

const type& variable_declaration::var_type() const
{
    return info_->var_type();
}

variable_intent variable_declaration::intent() const
{
    return info_->intent();
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
}