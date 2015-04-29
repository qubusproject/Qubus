#include <qbb/kubus/IR/local_variable_def_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

local_variable_def_expr::local_variable_def_expr(variable_declaration decl_,
                                                 expression initializer_, expression scope_)
: decl_(std::move(decl_)), initializer_(std::move(initializer_)), scope_(std::move(scope_))
{
}

const variable_declaration& local_variable_def_expr::decl() const
{
    return decl_;
}

const expression& local_variable_def_expr::initializer() const
{
    return initializer_;
}

const expression& local_variable_def_expr::scope() const
{
    return scope_;
}

std::vector<expression> local_variable_def_expr::sub_expressions() const
{
    return {initializer_, scope_};
}

expression
local_variable_def_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 2, "invalid number of subexpressions");

    return local_variable_def_expr(decl_, subexprs[0], subexprs[1]);
}

annotation_map& local_variable_def_expr::annotations() const
{
    return annotations_;
}

annotation_map& local_variable_def_expr::annotations()
{
    return annotations_;
}

bool operator==(const local_variable_def_expr& lhs, const local_variable_def_expr& rhs)
{
    return lhs.decl() == rhs.decl() && lhs.initializer() == rhs.initializer() && lhs.scope() == rhs.scope();
}

bool operator!=(const local_variable_def_expr& lhs, const local_variable_def_expr& rhs)
{
    return !(lhs == rhs);
}
}
}