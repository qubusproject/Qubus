#include <qbb/kubus/IR/compound_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

compound_expr::compound_expr(std::vector<expression> body_)
: body_(std::move(body_))
{
}
    
const std::vector<expression>& compound_expr::body() const
{
    return body_;
}

annotation_map& compound_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& compound_expr::annotations()
{
    return annotations_;
}

}
}