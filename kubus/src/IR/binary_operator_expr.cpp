#include <qbb/kubus/IR/binary_operator_expr.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

binary_operator_expr::binary_operator_expr(binary_op_tag tag_, expression left_, expression right_)
: tag_{tag_}, left_{std::move(left_)}, right_{std::move(right_)}
{
}

binary_op_tag binary_operator_expr::tag() const
{
    return tag_;
}

expression binary_operator_expr::left() const
{
    return left_;
}
expression binary_operator_expr::right() const
{
    return right_;
}

annotation_map& binary_operator_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& binary_operator_expr::annotations()
{
    return annotations_;
}

}
}