#include <qbb/kubus/IR/subscription_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
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

annotation_map& subscription_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& subscription_expr::annotations()
{
    return annotations_;
}

}
}