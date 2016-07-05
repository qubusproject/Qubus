#ifndef QBB_QUBUS_SUM_EXPR_HPP
#define QBB_QUBUS_SUM_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/vector.hpp>
#include <qbb/util/hpx/serialization/optional.hpp>

#include <boost/optional.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class sum_expr : public expression_base<sum_expr>
{
public:
    sum_expr() = default;
    sum_expr(variable_declaration contraction_index_, expression body_);
    sum_expr(std::vector<variable_declaration> contraction_indices_, expression body_);
    sum_expr(std::vector<variable_declaration> contraction_indices_, variable_declaration alias_, expression body_);

    expression body() const;
    
    const std::vector<variable_declaration>& contraction_indices() const;
    const boost::optional<variable_declaration>& alias() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & body_;
        ar & contraction_indices_;
        ar & alias_;
    }
private:
    expression body_;
    std::vector<variable_declaration> contraction_indices_;
    boost::optional<variable_declaration> alias_;
    
    mutable annotation_map annotations_;
};

bool operator==(const sum_expr& lhs, const sum_expr& rhs);
bool operator!=(const sum_expr& lhs, const sum_expr& rhs);

}
}

#endif