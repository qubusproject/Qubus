#include <qbb/kubus/IR/compound_expr.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

compound_expr::compound_expr(std::vector<expression> body_)
: body_(std::move(body_))
{
}
    
const std::vector<expression>& compound_expr::body() const
{
    return body_;
}

std::vector<expression> compound_expr::sub_expressions() const
{
    return body();
}

expression compound_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    return compound_expr(subexprs);
}

annotation_map& compound_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& compound_expr::annotations()
{
    return annotations_;
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