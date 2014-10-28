#ifndef QBB_KUBUS_VARIABLE_REF_EXPR_HPP
#define QBB_KUBUS_VARIABLE_REF_EXPR_HPP

#include <qbb/kubus/IR/annotations.hpp>

#include <memory>

namespace qbb
{
namespace kubus
{

class variable_declaration_expr;
    
class variable_ref_expr
{
public:
    explicit variable_ref_expr(variable_declaration_expr* declaration_);
    
    const variable_declaration_expr& declaration() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    variable_declaration_expr* declaration_;
    
    mutable annotation_map annotations_;
};
    
}
}

#endif