#include <qbb/kubus/IR/for_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

for_expr::for_expr(expression index_, expression body_)
: index_{std::move(index_)}, body_{std::move(body_)}
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