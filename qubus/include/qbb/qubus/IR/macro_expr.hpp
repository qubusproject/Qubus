#ifndef QBB_QUBUS_MACRO_EXPR_HPP
#define QBB_QUBUS_MACRO_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{
 
class macro_expr
{
public:
    macro_expr() = default;
    macro_expr(std::vector<variable_declaration> params_, expression body_);
    
    const std::vector<variable_declaration>& params() const;
    const expression& body() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & params_;
        ar & body_;
    }
private:
    std::vector<variable_declaration> params_;
    expression body_;
    
    mutable annotation_map annotations_;
};

bool operator==(const macro_expr& lhs, const macro_expr& rhs);
bool operator!=(const macro_expr& lhs, const macro_expr& rhs);

template<>
struct is_expression<macro_expr> : std::true_type
{
};

expression expand_macro(const macro_expr& macro, const std::vector<expression>& args);
    
}
}

#endif