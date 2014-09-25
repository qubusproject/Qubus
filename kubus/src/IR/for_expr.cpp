#include <qbb/kubus/IR/for_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

for_expr::for_expr(expression index_, expression lower_bound_, expression upper_bound_, expression body_)
: index_{std::move(index_)}, lower_bound_{std::move(lower_bound_)}, upper_bound_{std::move(upper_bound_)}, body_{std::move(body_)}
{
}

expression for_expr::body() const
{
    return body_;
}

expression for_expr::index() const
{
    return index_;
}

expression for_expr::lower_bound() const
{
    return lower_bound_;
}

expression for_expr::upper_bound() const
{
    return upper_bound_;
}

annotation_map& for_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& for_expr::annotations()
{
    return annotations_;
}

}
}