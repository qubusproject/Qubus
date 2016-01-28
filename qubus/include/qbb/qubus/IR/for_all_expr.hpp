#ifndef QBB_QUBUS_FOR_ALL_EXPR_HPP
#define QBB_QUBUS_FOR_ALL_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>

#include <boost/optional.hpp>

#include <hpx/runtime/serialization/vector.hpp>
#include <qbb/util/hpx/serialization/optional.hpp>

#include <vector>
#include <qbb/util/unused.hpp>

namespace qbb
{
namespace qubus
{
 
class for_all_expr
{
public:
    for_all_expr() = default;
    for_all_expr(variable_declaration loop_index_, expression body_);
    for_all_expr(std::vector<variable_declaration> loop_indices_, expression body_);
    for_all_expr(std::vector<variable_declaration> loop_indices_, variable_declaration alias_, expression body_);
    
    expression body() const;
    
    const std::vector<variable_declaration>& loop_indices() const;
    const boost::optional<variable_declaration>& alias() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & loop_indices_;
        ar & alias_;
        ar & body_;
    }
private:
    std::vector<variable_declaration> loop_indices_;
    boost::optional<variable_declaration> alias_;
    expression body_;
    
    mutable annotation_map annotations_;
};

bool operator==(const for_all_expr& lhs, const for_all_expr& rhs);
bool operator!=(const for_all_expr& lhs, const for_all_expr& rhs);

template<>
struct is_expression<for_all_expr> : std::true_type
{
};

}
}

#endif