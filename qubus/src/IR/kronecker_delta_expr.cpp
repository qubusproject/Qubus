#include <qbb/qubus/IR/kronecker_delta_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

kronecker_delta_expr::kronecker_delta_expr(util::index_t extent_, expression first_index_, expression second_index_)
: extent_{std::move(extent_)}, first_index_(std::move(first_index_)), second_index_(std::move(second_index_))
{
}

util::index_t kronecker_delta_expr::extent() const
{
    return extent_;
}

const expression& kronecker_delta_expr::first_index() const
{
    return first_index_;
}

const expression& kronecker_delta_expr::second_index() const
{
    return second_index_;
}

std::vector<expression> kronecker_delta_expr::sub_expressions() const
{
    return { first_index_, second_index_ };
}

expression
kronecker_delta_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 2, "invalid number of subexpressions");

    return kronecker_delta_expr(extent(), subexprs[0], subexprs[1]);
}

annotation_map& kronecker_delta_expr::annotations() const
{
    return annotations_;
}

annotation_map& kronecker_delta_expr::annotations()
{
    return annotations_;
}

bool operator==(const kronecker_delta_expr& lhs, const kronecker_delta_expr& rhs)
{
    return lhs.extent() == rhs.extent() && lhs.first_index() == rhs.first_index() &&
           lhs.second_index() == rhs.second_index();
}

bool operator!=(const kronecker_delta_expr& lhs, const kronecker_delta_expr& rhs)
{
    return !(lhs == rhs);
}
}
}
