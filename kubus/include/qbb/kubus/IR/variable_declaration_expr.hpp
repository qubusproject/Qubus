#ifndef QBB_KUBUS_VARIABLE_DECLARATION_EXPR_HPP
#define QBB_KUBUS_VARIABLE_DECLARATION_EXPR_HPP

#include <qbb/kubus/IR/type.hpp>

#include <qbb/kubus/IR/annotations.hpp>

#include <memory>

namespace qbb
{
namespace kubus
{

class variable_declaration_expr
{
public:
    explicit variable_declaration_expr(type var_type_);
    
    const type& var_type() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    type var_type_;
    
    mutable annotation_map annotations_;
};
    
}
}

#endif