#ifndef QUBUS_ACCESS_HPP
#define QUBUS_ACCESS_HPP

#include <qbb/qubus/IR/expression.hpp>

inline namespace qbb
{
namespace qubus
{

class access_expr : public expression
{
public:
    virtual ~access_expr() = default;
};

template<typename AccessExpression, typename AccessExpressionBase = access_expr>
class access_expr_base :  public expression_base<AccessExpression, AccessExpressionBase>
{
public:
    virtual ~access_expr_base() = default;
};

}
}

#endif
