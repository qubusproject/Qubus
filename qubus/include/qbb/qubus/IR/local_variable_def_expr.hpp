#ifndef QBB_QUBUS_LOCAL_VARIABLE_DEF_EXPR_HPP
#define QBB_QUBUS_LOCAL_VARIABLE_DEF_EXPR_HPP

#include <qbb/qubus/IR/variable_declaration.hpp>
#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class local_variable_def_expr
{
public:
    local_variable_def_expr() = default;
    local_variable_def_expr(variable_declaration decl_, expression initializer_);

    const variable_declaration& decl() const;
    const expression& initializer() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;

    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & decl_;
        ar & initializer_;
    }
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