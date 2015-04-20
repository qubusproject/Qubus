#include <qbb/kubus/IR/variable_ref_expr.hpp>

#include <qbb/util/assert.hpp>
#include <qbb/util/unused.hpp>

namespace qbb
{
namespace kubus
{

variable_ref_expr::variable_ref_expr(variable_declaration declaration_) : declaration_(declaration_)
{
}

const variable_declaration& variable_ref_expr::declaration() const
{
    return declaration_;
}

std::vector<expression> variable_ref_expr::sub_expressions() const
{
    return {};
}

expression variable_ref_expr::substitute_subexpressions(
    const std::vector<expression>& QBB_UNUSED_RELEASE(subexprs)) const
{
    QBB_ASSERT(subexprs.size() == 0, "invalid number of subexpressions");

    return variable_ref_expr(declaration_);
}

annotation_map& variable_ref_expr::annotations() const
{
    return annotations_;
}

annotation_map& variable_ref_expr::annotations()
{
    return annotations_;
}

bool operator==(const variable_ref_expr& lhs, const variable_ref_expr& rhs)
{
    return lhs.declaration() == rhs.declaration();
}

bool operator!=(const variable_ref_expr& lhs, const variable_ref_expr& rhs)
{
    return !(lhs == rhs);
}
}
}