#ifndef QBB_QUBUS_INTRINSIC_FUNCTION_EXPR_HPP
#define QBB_QUBUS_INTRINSIC_FUNCTION_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/string.hpp>
#include <hpx/runtime/serialization/vector.hpp>

#include <string>
#include <vector>

namespace qbb
{
namespace qubus
{

class intrinsic_function_expr
{
public:
    intrinsic_function_expr() = default;
    intrinsic_function_expr(std::string name_, std::vector<expression> args_);

    const std::string& name() const;

    const std::vector<expression>& args() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & name_;
        ar & args_;
    }
private:
    std::string name_;
    std::vector<expression> args_;
    
    mutable annotation_map annotations_;
};

bool operator==(const intrinsic_function_expr& lhs, const intrinsic_function_expr& rhs);
bool operator!=(const intrinsic_function_expr& lhs, const intrinsic_function_expr& rhs);

template<>
struct is_expression<intrinsic_function_expr> : std::true_type
{
};

}
}

#endif