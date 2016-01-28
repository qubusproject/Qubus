#ifndef QBB_QUBUS_TYPE_CONVERSION_EXPR_HPP
#define QBB_QUBUS_TYPE_CONVERSION_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/type.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class type_conversion_expr
{
public:
    type_conversion_expr() = default;
    type_conversion_expr(type target_type_, expression arg_);

    type target_type() const;

    expression arg() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & target_type_;
        ar & arg_;
    }
private:
    type target_type_;
    expression arg_;
    
    mutable annotation_map annotations_;
};

bool operator==(const type_conversion_expr& lhs, const type_conversion_expr& rhs);
bool operator!=(const type_conversion_expr& lhs, const type_conversion_expr& rhs);

template<>
struct is_expression<type_conversion_expr> : std::true_type
{
};

}
}

#endif