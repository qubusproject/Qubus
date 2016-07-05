#ifndef QBB_QUBUS_SPAWN_EXPR_HPP
#define QBB_QUBUS_SPAWN_EXPR_HPP

#include <qbb/qubus/IR/function_declaration.hpp>
#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class spawn_expr : public expression_base<spawn_expr>
{
public:
    spawn_expr() = default;
    spawn_expr(function_declaration spawned_plan_, std::vector<expression> arguments_);
    
    const function_declaration& spawned_plan() const;
    const std::vector<expression>& arguments() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & spawned_plan_;
        ar & arguments_;
    }
private:   
    function_declaration spawned_plan_;
    std::vector<expression> arguments_;
    
    mutable annotation_map annotations_;
};

bool operator==(const spawn_expr& lhs, const spawn_expr& rhs);
bool operator!=(const spawn_expr& lhs, const spawn_expr& rhs);

}
}

#endif