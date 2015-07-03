#include <qbb/qubus/IR/for_all_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

for_all_expr::for_all_expr(variable_declaration loop_index_, expression body_)
: loop_indices_{std::move(loop_index_)}, body_(std::move(body_))
{
}

for_all_expr::for_all_expr(std::vector<variable_declaration> loop_indices_, expression body_)
: loop_indices_(std::move(loop_indices_)), body_(std::move(body_))
{
    QBB_ASSERT(this->loop_indices_.size() > 0, "A for all loops needs to declare at least one index.");
}

for_all_expr::for_all_expr(std::vector<variable_declaration> loop_indices_, variable_declaration alias_, expression body_)
: loop_indices_(std::move(loop_indices_)), alias_(std::move(alias_)), body_(std::move(body_))
{
    QBB_ASSERT(this->loop_indices_.size() > 0, "A for all loops needs to declare at least one index.");
}

expression for_all_expr::body() const
{
    return body_;
}

const std::vector<variable_declaration>& for_all_expr::loop_indices() const
{
    return loop_indices_;
}

const boost::optional<variable_declaration>& for_all_expr::alias() const
{
    return alias_;
}

std::vector<expression> for_all_expr::sub_expressions() const
{
    return {body_};
}

expression for_all_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 1, "invalid number of subexpressions");

    if (alias())
    {
        return for_all_expr(loop_indices_, *alias(), subexprs[0]);
    }
    else
    {
        return for_all_expr(loop_indices_, subexprs[0]);
    }
}

annotation_map& for_all_expr::annotations() const
{
    return annotations_;
}

annotation_map& for_all_expr::annotations()
{
    return annotations_;
}

bool operator==(const for_all_expr& lhs, const for_all_expr& rhs)
{
    return lhs.loop_indices() == rhs.loop_indices() && lhs.body() == rhs.body();
}

bool operator!=(const for_all_expr& lhs, const for_all_expr& rhs)
{
    return !(lhs == rhs);
}

}
}