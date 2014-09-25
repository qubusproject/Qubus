#ifndef QBB_KUBUS_FOR_EXPR_HPP
#define QBB_KUBUS_FOR_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>

namespace qbb
{
namespace kubus
{

class for_expr
{
public:
    for_expr(expression index_, expression lower_bound_, expression upper_bound_, expression body_);

    expression body() const;

    expression index() const;

    expression lower_bound() const;
    expression upper_bound() const;

    annotation_map& annotations() const;
    annotation_map& annotations();

private:
    expression index_;
    expression lower_bound_;
    expression upper_bound_;
    expression body_;

    mutable annotation_map annotations_;
};
}
}

#endif