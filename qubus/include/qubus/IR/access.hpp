#ifndef QUBUS_ACCESS_HPP
#define QUBUS_ACCESS_HPP

#include <qubus/IR/expression.hpp>

#include <memory>

namespace qubus
{

class access_expr : public expression
{
public:
    virtual ~access_expr() = default;

    virtual access_expr* clone() const override = 0;
};

template<typename AccessExpression, typename AccessExpressionBase = access_expr>
class access_expr_base :  public expression_base<AccessExpression, AccessExpressionBase>
{
public:
    virtual ~access_expr_base() = default;
};

inline std::unique_ptr<access_expr> clone(const access_expr& expr)
{
    return std::unique_ptr<access_expr>(expr.clone());
}

}

#endif
