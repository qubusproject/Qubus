#include <qbb/qubus/IR/compound_expr.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

compound_expr::compound_expr(std::vector<std::unique_ptr<expression>> body_)
: body_(take_over_children(body_))
{
}

compound_expr* compound_expr::clone() const
{
    return new compound_expr(qbb::qubus::clone(body_));
}

const expression& compound_expr::child(std::size_t index) const
{
    return *body_.at(index);
}

std::size_t compound_expr::arity() const
{
    return body_.size();
}

std::unique_ptr<expression> compound_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    return std::make_unique<compound_expr>(std::move(new_children));
}

bool operator==(const compound_expr& lhs, const compound_expr& rhs)
{
    return lhs.body() == rhs.body();
}

bool operator!=(const compound_expr& lhs, const compound_expr& rhs)
{
    return !(lhs == rhs);
}
}
}