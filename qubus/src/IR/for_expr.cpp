#include <qbb/qubus/IR/for_expr.hpp>

#include <qbb/qubus/IR/literal_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

for_expr::for_expr(variable_declaration loop_index_, expression lower_bound_,
                   expression upper_bound_, expression body_)
: for_expr(std::move(loop_index_), std::move(lower_bound_), std::move(upper_bound_),
           integer_literal_expr(1), std::move(body_))
{
}

for_expr::for_expr(variable_declaration loop_index_, expression lower_bound_,
                   expression upper_bound_, expression increment_, expression body_)
: loop_index_(std::move(loop_index_)), lower_bound_(std::move(lower_bound_)),
  upper_bound_(std::move(upper_bound_)), increment_(std::move(increment_)), body_(std::move(body_))
{
}

expression for_expr::body() const
{
    return body_;
}

const variable_declaration& for_expr::loop_index() const
{
    return loop_index_;
}

expression for_expr::lower_bound() const
{
    return lower_bound_;
}

expression for_expr::upper_bound() const
{
    return upper_bound_;
}

expression for_expr::increment() const
{
    return increment_;
}

std::vector<expression> for_expr::sub_expressions() const
{
    return {lower_bound_, upper_bound_, increment_, body_};
}

expression for_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 4, "invalid number of subexpressions");

    return for_expr(loop_index_, subexprs[0], subexprs[1], subexprs[2], subexprs[3]);
}

annotation_map& for_expr::annotations() const
{
    return annotations_;
}

annotation_map& for_expr::annotations()
{
    return annotations_;
}

bool operator==(const for_expr& lhs, const for_expr& rhs)
{
    return lhs.loop_index() == rhs.loop_index() && lhs.lower_bound() == rhs.lower_bound() &&
           lhs.upper_bound() == rhs.upper_bound() && lhs.increment() == rhs.increment() &&
           lhs.body() == rhs.body();
}

bool operator!=(const for_expr& lhs, const for_expr& rhs)
{
    return !(lhs == rhs);
}
}
}