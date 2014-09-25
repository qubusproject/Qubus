#include <qbb/kubus/IR/sum_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

sum_expr::sum_expr(expression body_, expression index_)
: body_{std::move(body_)}, indices_{std::move(index_)}
{
}

sum_expr::sum_expr(expression body_, std::vector<expression> indices_)
: body_{std::move(body_)}, indices_(std::move(indices_))
{
}

expression sum_expr::body() const
{
    return body_;
}

const std::vector<expression>& sum_expr::indices() const
{
    return indices_;
}

annotation_map& sum_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& sum_expr::annotations()
{
    return annotations_;
}

}
}