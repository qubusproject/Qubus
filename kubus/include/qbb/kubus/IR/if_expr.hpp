#ifndef QBB_KUBUS_IF_EXPR_HPP
#define QBB_KUBUS_IF_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/variable_declaration.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>

#include <boost/optional.hpp>

namespace qbb
{
namespace qubus
{

class if_expr
{
public:
    if_expr(expression condition_, expression then_branch_);
    if_expr(expression condition_, expression then_branch_, expression else_branch_);

    expression condition() const;

    expression then_branch() const;
    boost::optional<expression> else_branch() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;

    annotation_map& annotations() const;
    annotation_map& annotations();

private:
    expression condition_;
    expression then_branch_;
    boost::optional<expression> else_branch_;

    mutable annotation_map annotations_;
};

bool operator==(const if_expr& lhs, const if_expr& rhs);
bool operator!=(const if_expr& lhs, const if_expr& rhs);

template<>
struct is_expression<if_expr> : std::true_type
{
};

}
}

#endif
