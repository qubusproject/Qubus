#include <qbb/kubus/IR/sum_expr.hpp>

#include <qbb/kubus/IR/type.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

sum_expr::sum_expr(variable_declaration index_decl_, expression body_)
: body_(std::move(body_)), index_decl_(std::move(index_decl_))
{
}

expression sum_expr::body() const
{
    return body_;
}

const variable_declaration& sum_expr::index_decl() const
{
    return index_decl_;
}

std::vector<expression> sum_expr::sub_expressions() const
{
    return {body_};
}

expression sum_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 1, "invalid number of subexpressions");
    
    return sum_expr(index_decl_, subexprs[0]);
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
    return lhs.index_decl() == rhs.index_decl() && lhs.body() == rhs.body();
}

bool operator!=(const sum_expr& lhs, const sum_expr& rhs)
{
    return !(lhs == rhs);
}

}
}