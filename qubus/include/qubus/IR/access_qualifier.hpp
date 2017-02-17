#ifndef QUBUS_ACCESS_QUALIFIER_HPP
#define QUBUS_ACCESS_QUALIFIER_HPP

#include <qubus/IR/access.hpp>

namespace qubus
{

class access_qualifier_expr : public access_expr
{
public:
    virtual ~access_qualifier_expr() = default;

    virtual const access_expr& qualified_access() const = 0;
};

template<typename AccessQualifierExpression, typename AccessQualifierExpressionBase = access_qualifier_expr>
class access_qualifier_base :  public access_expr_base<AccessQualifierExpression, AccessQualifierExpressionBase>
{
public:
    virtual ~access_qualifier_base() = default;
};

}

#endif
