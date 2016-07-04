#ifndef QBB_QUBUS_SUBSCRIPTION_EXPR_HPP
#define QBB_QUBUS_SUBSCRIPTION_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/vector.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class subscription_expr
{
public:
    subscription_expr() = default;
    subscription_expr(expression indexed_expr_, std::vector<expression> indices_);
    
    expression indexed_expr() const;
    
    const std::vector<expression>& indices() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & indexed_expr_;
        ar & indices_;
    }
private:
    expression indexed_expr_;
    std::vector<expression> indices_;
    
    mutable annotation_map annotations_;
};

bool operator==(const subscription_expr& lhs, const subscription_expr& rhs);
bool operator!=(const subscription_expr& lhs, const subscription_expr& rhs);

template<>
struct is_expression<subscription_expr> : std::true_type
{
};

}
}

#endif