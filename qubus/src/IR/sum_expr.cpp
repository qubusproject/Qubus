#include <qbb/qubus/IR/sum_expr.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

sum_expr::sum_expr(variable_declaration contraction_index_, expression body_)
: body_(std::move(body_)), contraction_indices_{std::move(contraction_index_)}
{
}

sum_expr::sum_expr(std::vector<variable_declaration> contraction_indices_, expression body_)
: body_(std::move(body_)), contraction_indices_(std::move(contraction_indices_))
{
}

sum_expr::sum_expr(std::vector<variable_declaration> contraction_indices_,
                   variable_declaration alias_, expression body_)
: body_(std::move(body_)), contraction_indices_(std::move(contraction_indices_)),
  alias_(std::move(alias_))
{
}

expression sum_expr::body() const
{
    return body_;
}

const std::vector<variable_declaration>& sum_expr::contraction_indices() const
{
    return contraction_indices_;
}

const boost::optional<variable_declaration>& sum_expr::alias() const
{
    return alias_;
}

std::vector<expression> sum_expr::sub_expressions() const
{
    return {body_};
}

expression sum_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 1, "invalid number of subexpressions");

    if (alias())
    {
        return sum_expr(contraction_indices_, *alias(), subexprs[0]);
    }
    else
    {
        return sum_expr(contraction_indices_, subexprs[0]);
    }
}

annotation_map& sum_expr::annotations() const
{
    return annotations_;
}

annotation_map& sum_expr::annotations()
{
    return annotations_;
}

bool operator==(const sum_expr& lhs, const sum_expr& rhs)
{
    return lhs.contraction_indices() == rhs.contraction_indices() && lhs.body() == rhs.body();
}

bool operator!=(const sum_expr& lhs, const sum_expr& rhs)
{
    return !(lhs == rhs);
}
}
}