#include <qbb/kubus/IR/scoped_view_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
scoped_view_expr::scoped_view_expr(variable_declaration view_var_,
                                   variable_declaration referenced_var_,
                                   std::vector<expression> origin_,
                                   std::vector<util::index_t> shape_, bool is_mutable_,
                                   expression body_)
: view_var_(std::move(view_var_)), referenced_var_(std::move(referenced_var_)),
  origin_(std::move(origin_)), shape_(std::move(shape_)), is_mutable_(is_mutable_),
  body_(std::move(body_))
{
}

scoped_view_expr::scoped_view_expr(variable_declaration view_var_,
                                   variable_declaration referenced_var_,
                                   std::vector<expression> origin_,
                                   std::vector<util::index_t> shape_,
                                   std::vector<util::index_t> permutation_, bool is_mutable_,
                                   expression body_)
: view_var_(std::move(view_var_)), referenced_var_(std::move(referenced_var_)),
  origin_(std::move(origin_)), shape_(std::move(shape_)), permutation_(std::move(permutation_)),
  is_mutable_(is_mutable_), body_(std::move(body_))
{
}

const variable_declaration& scoped_view_expr::view_var() const
{
    return view_var_;
}

const variable_declaration& scoped_view_expr::referenced_var() const
{
    return referenced_var_;
}

const std::vector<expression>& scoped_view_expr::origin() const
{
    return origin_;
}

const std::vector<util::index_t>& scoped_view_expr::shape() const
{
    return shape_;
}

const boost::optional<std::vector<util::index_t>>& scoped_view_expr::permutation() const
{
    return permutation_;
}

bool scoped_view_expr::is_mutable() const
{
    return is_mutable_;
}

const expression& scoped_view_expr::body() const
{
    return body_;
}

std::vector<expression> scoped_view_expr::sub_expressions() const
{
    return {body_};
}

expression
scoped_view_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 1, "invalid number of subexpressions");

    if (!permutation_)
    {
        return scoped_view_expr(view_var_, referenced_var_, origin_, shape_, is_mutable_,
                                subexprs[0]);
    }
    else
    {
        return scoped_view_expr(view_var_, referenced_var_, origin_, shape_, *permutation_,
                                is_mutable_, subexprs[0]);
    }
}

annotation_map& scoped_view_expr::annotations() const
{
    return annotations_;
}

annotation_map& scoped_view_expr::annotations()
{
    return annotations_;
}
}
}