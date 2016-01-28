#ifndef QBB_QUBUS_IF_EXPR_HPP
#define QBB_QUBUS_IF_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <qbb/util/hpx/serialization/optional.hpp>

#include <boost/optional.hpp>

namespace qbb
{
namespace qubus
{

class if_expr
{
public:
    if_expr() = default;
    if_expr(expression condition_, expression then_branch_);
    if_expr(expression condition_, expression then_branch_, expression else_branch_);

    expression condition() const;

    expression then_branch() const;
    boost::optional<expression> else_branch() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;

    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & condition_;
        ar & then_branch_;
        ar & else_branch_;
    }
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
