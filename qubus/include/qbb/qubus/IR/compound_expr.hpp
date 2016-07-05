#ifndef QBB_QUBUS_COMPOUND_EXPR_HPP
#define QBB_QUBUS_COMPOUND_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/vector.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{
 
class compound_expr : public expression_base<compound_expr>
{
public:
    compound_expr() = default;
    compound_expr(std::vector<expression> body_);
    
    const std::vector<expression>& body() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & body_;
    }
private:
    std::vector<expression> body_;
    
    mutable annotation_map annotations_;
};

bool operator==(const compound_expr& lhs, const compound_expr& rhs);
bool operator!=(const compound_expr& lhs, const compound_expr& rhs);

}
}

#endif