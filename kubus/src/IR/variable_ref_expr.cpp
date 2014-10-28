#include <qbb/kubus/IR/variable_ref_expr.hpp>

namespace qbb
{
namespace kubus
{

variable_ref_expr::variable_ref_expr(variable_declaration_expr* declaration_)
: declaration_(declaration_)
{
}

const variable_declaration_expr& variable_ref_expr::declaration() const
{
    return *declaration_;
}

annotation_map& variable_ref_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& variable_ref_expr::annotations()
{
    return annotations_;
}

}
}