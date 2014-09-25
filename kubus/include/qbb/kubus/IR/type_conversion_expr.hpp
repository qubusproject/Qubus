#ifndef QBB_KUBUS_TYPE_CONVERSION_EXPR_HPP
#define QBB_KUBUS_TYPE_CONVERSION_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/type.hpp>

namespace qbb
{
namespace kubus
{

class type_conversion_expr
{
public:
    type_conversion_expr(type target_type_, expression arg_);

    type target_type() const;

    expression arg() const;

    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    type target_type_;
    expression arg_;
    
    mutable annotation_map annotations_;
};
}
}

#endif