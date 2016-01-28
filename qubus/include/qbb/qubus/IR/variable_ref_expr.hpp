#ifndef QBB_QUBUS_VARIABLE_REF_EXPR_HPP
#define QBB_QUBUS_VARIABLE_REF_EXPR_HPP

#include <qbb/qubus/IR/variable_declaration.hpp>
#include <qbb/qubus/IR/annotations.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>

#include <qbb/util/hash.hpp>
#include <qbb/util/unused.hpp>

#include <memory>
#include <vector>
#include <functional>

namespace qbb
{
namespace qubus
{
    
class variable_ref_expr
{
public:
    variable_ref_expr() = default;
    explicit variable_ref_expr(variable_declaration declaration_);
    
    const variable_declaration& declaration() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & declaration_;
    }
private:
    variable_declaration declaration_;
    
    mutable annotation_map annotations_;
};

bool operator==(const variable_ref_expr& lhs, const variable_ref_expr& rhs);
bool operator!=(const variable_ref_expr& lhs, const variable_ref_expr& rhs);

template<>
struct is_expression<variable_ref_expr> : std::true_type
{
};

}
}

#endif