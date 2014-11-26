#include <qbb/kubus/IR/for_all_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

for_all_expr::for_all_expr(variable_declaration loop_index_, expression body_)
: loop_index_(std::move(loop_index_)), body_(std::move(body_))
{
}

expression for_all_expr::body() const
{
    return body_;
}

const variable_declaration& for_all_expr::loop_index() const
{
    return loop_index_;
}

std::vector<expression> for_all_expr::sub_expressions() const
{
    return {body_};
}

expression for_all_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 1, "invalid number of subexpressions");
    
    return for_all_expr(loop_index_, subexprs[0]);
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
    return lhs.loop_index() == rhs.loop_index() && lhs.body() == rhs.body();
}

bool operator!=(const for_all_expr& lhs, const for_all_expr& rhs)
{
    return !(lhs == rhs);
}

}
}