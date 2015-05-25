#ifndef QBB_KUBUS_LOCAL_VARIABLE_DEF_EXPR_HPP
#define QBB_KUBUS_LOCAL_VARIABLE_DEF_EXPR_HPP

#include <qbb/kubus/IR/variable_declaration.hpp>
#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{

class local_variable_def_expr
{
public:
    local_variable_def_expr(variable_declaration decl_, expression initializer_);

    const variable_declaration& decl() const;
    const expression& initializer() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;

    annotation_map& annotations() const;
    annotation_map& annotations();

private:
    variable_declaration decl_;
    expression initializer_;

    mutable annotation_map annotations_;
};

bool operator==(const local_variable_def_expr& lhs, const local_variable_def_expr& rhs);
bool operator!=(const local_variable_def_expr& lhs, const local_variable_def_expr& rhs);

template <>
struct is_expression<local_variable_def_expr> : std::true_type
{
};
}
}

#endif