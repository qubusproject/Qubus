#include <qbb/kubus/IR/index_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

index_expr::index_expr(std::string id_)
:id_{std::move(id_)}
{
}

std::string index_expr::id() const
{
    return id_;
}

annotation_map& index_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& index_expr::annotations()
{
    return annotations_;
}

}
}