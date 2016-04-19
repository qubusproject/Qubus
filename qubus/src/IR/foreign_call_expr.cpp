#include <qbb/qubus/IR/foreign_call_expr.hpp>

#include <qbb/util/unused.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

foreign_call_expr::foreign_call_expr(foreign_computelet computelet_, std::vector<expression> args_)
: computelet_(std::move(computelet_)), args_(std::move(args_))
{
}

type foreign_call_expr::result_type() const
{
    return computelet_.result_type();
}

const foreign_computelet& foreign_call_expr::computelet() const
{
    return computelet_;
}

const std::vector<expression>& foreign_call_expr::args() const
{
    return args_;
}

std::vector<expression> foreign_call_expr::sub_expressions() const
{
    return args_;
}

expression foreign_call_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    return foreign_call_expr(computelet_, subexprs);
}

annotation_map& foreign_call_expr::annotations() const
{
    return annotations_;
}

annotation_map& foreign_call_expr::annotations()
{
    return annotations_;
}

bool operator==(const foreign_call_expr& QBB_UNUSED(lhs), const foreign_call_expr& QBB_UNUSED(rhs))
{
    return false;
}

bool operator!=(const foreign_call_expr& lhs, const foreign_call_expr& rhs)
{
    return !(lhs == rhs);
}

}
}
