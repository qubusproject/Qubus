#ifndef QBB_KUBUS_SCOPED_VIEW_EXPR_HPP
#define QBB_KUBUS_SCOPED_VIEW_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>
#include <qbb/kubus/IR/variable_declaration.hpp>

#include <qbb/util/integers.hpp>

#include <boost/optional.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{
class scoped_view_expr
{
public:
    scoped_view_expr(variable_declaration view_var_, variable_declaration referenced_var_,
                     std::vector<expression> origin_, std::vector<util::index_t> shape_,
                     expression body_);

    scoped_view_expr(variable_declaration view_var_, variable_declaration referenced_var_,
                     std::vector<expression> origin_, std::vector<util::index_t> shape_,
                     std::vector<util::index_t> permutation_, expression body_);

    const variable_declaration& view_var() const;
    const variable_declaration& referenced_var() const;
    const std::vector<expression>& origin() const;
    const std::vector<util::index_t>& shape() const;
    const boost::optional<std::vector<util::index_t>>& permutation() const;
    const expression& body() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;

    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    variable_declaration view_var_;
    variable_declaration referenced_var_;
    std::vector<expression> origin_;
    std::vector<util::index_t> shape_;
    boost::optional<std::vector<util::index_t>> permutation_;
    expression body_;

    mutable annotation_map annotations_;
};

template <>
struct is_expression<scoped_view_expr> : std::true_type
{
};
}
}

#endif