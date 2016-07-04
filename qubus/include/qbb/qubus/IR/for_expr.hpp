#ifndef QBB_QUBUS_FOR_EXPR_HPP
#define QBB_QUBUS_FOR_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class for_expr
{
public:
    for_expr() = default;
    for_expr(variable_declaration loop_index_, expression lower_bound_, expression upper_bound_, expression body_);
    for_expr(variable_declaration loop_index_, expression lower_bound_, expression upper_bound_, expression increment_, expression body_);
    
    expression body() const;

    const variable_declaration& loop_index() const;

    expression lower_bound() const;
    expression upper_bound() const;
    expression increment() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & loop_index_;
        ar & lower_bound_;
        ar & upper_bound_;
        ar & increment_;
        ar & body_;
    }
private:
    variable_declaration loop_index_;
    expression lower_bound_;
    expression upper_bound_;
    expression increment_;
    expression body_;

    mutable annotation_map annotations_;
};

bool operator==(const for_expr& lhs, const for_expr& rhs);
bool operator!=(const for_expr& lhs, const for_expr& rhs);

template<>
struct is_expression<for_expr> : std::true_type
{
};

}
}

#endif