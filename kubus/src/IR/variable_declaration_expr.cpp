#include <qbb/kubus/IR/variable_declaration_expr.hpp>

namespace qbb
{
namespace kubus
{

variable_declaration_expr::variable_declaration_expr(type var_type_)
: var_type_(var_type_)
{
}

const type& variable_declaration_expr::var_type() const
{
    return var_type_;
}

annotation_map& variable_declaration_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& variable_declaration_expr::annotations()
{
    return annotations_;
}

}
}