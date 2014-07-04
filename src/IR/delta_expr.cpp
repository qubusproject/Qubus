#include <qbb/kubus/IR/delta_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

delta_expr::delta_expr(std::vector<expression> indices_)
: indices_(std::move(indices_))
{
}
    
const std::vector<expression>& delta_expr::indices() const
{
    return indices_;
}
    
annotation_map& delta_expr::annotations() const
{
    return annotations_;
}

annotation_map& delta_expr::annotations()
{
    return annotations_;
}
    
}  
}