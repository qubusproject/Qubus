#include <qbb/kubus/IR/literal_expr.hpp>

namespace qbb
{
namespace kubus
{
    
double_literal_expr::double_literal_expr(double value_)
:value_{value_}
{
}
    
double double_literal_expr::value() const
{
    return value_;
}

annotation_map& double_literal_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& double_literal_expr::annotations()
{
    return annotations_;
}

float_literal_expr::float_literal_expr(float value_)
:value_{value_}
{
}
    
float float_literal_expr::value() const
{
    return value_;
}

annotation_map& float_literal_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& float_literal_expr::annotations()
{
    return annotations_;
}

integer_literal_expr::integer_literal_expr(qbb::util::index_t value_)
:value_{value_}
{
}
    
qbb::util::index_t integer_literal_expr::value() const
{
    return value_;
}

annotation_map& integer_literal_expr::annotations() const
{
    return annotations_;
}
    
annotation_map& integer_literal_expr::annotations()
{
    return annotations_;
}

}
}