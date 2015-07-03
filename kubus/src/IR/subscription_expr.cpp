#include <qbb/kubus/IR/subscription_expr.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

subscription_expr::subscription_expr(expression indexed_expr_, std::vector<expression> indices_)
: indexed_expr_{std::move(indexed_expr_)}, indices_(std::move(indices_))
{
}

expression subscription_expr::indexed_expr() const
{
    return indexed_expr_;
}

const std::vector<expression>& subscription_expr::indices() const
{
    return indices_;
}

std::vector<expression> subscription_expr::sub_expressions() const
{
    std::vector<expression> sub_exprs;
    sub_exprs.reserve(indices_.size() + 1);
    
    sub_exprs.push_back(indexed_expr_);

    sub_exprs.insert(sub_exprs.end(), indices_.begin(), indices_.end());
    
    return sub_exprs;
}

expression subscription_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    std::vector<expression> new_indices;
    
    for(std::size_t i = 1; i < subexprs.size(); ++i)
    {
        new_indices.emplace_back(subexprs[i]);
    }
    
    return subscription_expr(subexprs[0], new_indices);
}

annotation_map& subscription_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& subscription_expr::annotations()
{
    return annotations_;
}

bool operator==(const subscription_expr& lhs, const subscription_expr& rhs)
{
    return lhs.indexed_expr() == rhs.indexed_expr() && lhs.indices() == rhs.indices();
}

bool operator!=(const subscription_expr& lhs, const subscription_expr& rhs)
{
    return !(lhs == rhs);
}

}
}