#include <qubus/qtl/IR/multi_index_expr.hpp>

#include <utility>

namespace qubus
{
namespace qtl
{

multi_index_expr::multi_index_expr(variable_declaration multi_index_,
                                   std::vector<variable_declaration> element_indices_)
: multi_index_(std::move(multi_index_)), element_indices_(std::move(element_indices_))
{
}

const variable_declaration& multi_index_expr::multi_index() const
{
    return multi_index_;
}

const std::vector<variable_declaration>& multi_index_expr::element_indices() const
{
    return element_indices_;
}

multi_index_expr* multi_index_expr::clone() const
{
    return new multi_index_expr(multi_index_, element_indices_);
}

const expression& multi_index_expr::child(std::size_t index) const
{
    throw 0;
}

std::size_t multi_index_expr::arity() const
{
    return 0;
}

std::unique_ptr<expression>
multi_index_expr::substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const
{
    if (!new_children.empty())
        throw 0;

    return ::qubus::clone(*this);
}

bool operator==(const multi_index_expr& lhs, const multi_index_expr& rhs)
{
    return lhs.multi_index() == rhs.multi_index() && lhs.element_indices() == rhs.element_indices();
}

bool operator!=(const multi_index_expr& lhs, const multi_index_expr& rhs)
{
    return !(lhs == rhs);
}
}
}
