#ifndef QBB_KUBUS_SPAWN_EXPR_HPP
#define QBB_KUBUS_SPAWN_EXPR_HPP

#include <qbb/kubus/IR/function_declaration.hpp>
#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{

class spawn_expr
{
public:
    spawn_expr(function_declaration spawned_plan_, std::vector<expression> arguments_);
    
    const function_declaration& spawned_plan() const;
    const std::vector<expression>& arguments() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:   
    function_declaration spawned_plan_;
    std::vector<expression> arguments_;
    
    mutable annotation_map annotations_;
};

bool operator==(const spawn_expr& lhs, const spawn_expr& rhs);
bool operator!=(const spawn_expr& lhs, const spawn_expr& rhs);

template<>
struct is_expression<spawn_expr> : std::true_type
{
};

}
}

#endif