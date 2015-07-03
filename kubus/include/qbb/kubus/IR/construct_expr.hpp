#ifndef QBB_QUBUS_CONSTRUCT_EXPR_HPP
#define QBB_QUBUS_CONSTRUCT_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>
#include <qbb/kubus/IR/type.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class construct_expr
{
public:
    construct_expr(type result_type_, std::vector<expression> parameters_);

    const type& result_type() const;

    const std::vector<expression>& parameters() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;

    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    type result_type_;
    std::vector<expression> parameters_;

    mutable annotation_map annotations_;
};

bool operator==(const construct_expr& lhs, const construct_expr& rhs);
bool operator!=(const construct_expr& lhs, const construct_expr& rhs);

template<>
struct is_expression<construct_expr> : std::true_type
{
};

}
}

#endif
