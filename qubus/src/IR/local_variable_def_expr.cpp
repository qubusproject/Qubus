#include <qbb/qubus/IR/local_variable_def_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

local_variable_def_expr::local_variable_def_expr(variable_declaration decl_,
                                                 std::unique_ptr<expression> initializer_)
: decl_(std::move(decl_)), initializer_(take_over_child(initializer_))
{
}

const variable_declaration& local_variable_def_expr::decl() const
{
    return decl_;
}

const expression& local_variable_def_expr::initializer() const
{
    return *initializer_;
}

local_variable_def_expr* local_variable_def_expr::clone() const
{
    return new local_variable_def_expr(decl_, qbb::qubus::clone(*initializer_));
}

const expression& local_variable_def_expr::child(std::size_t index) const
{
    if (index == 0)
    {
        return *initializer_;
    }
    else
    {
        throw 0;
    }
}

std::size_t local_variable_def_expr::arity() const
{
    return 1;
}

std::unique_ptr<expression> local_variable_def_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 1)
        throw 0;

    return std::make_unique<local_variable_def_expr>(decl_, std::move(new_children[0]));
}

bool operator==(const local_variable_def_expr& lhs, const local_variable_def_expr& rhs)
{
    return lhs.decl() == rhs.decl() && lhs.initializer() == rhs.initializer();
}

bool operator!=(const local_variable_def_expr& lhs, const local_variable_def_expr& rhs)
{
    return !(lhs == rhs);
}
}
}