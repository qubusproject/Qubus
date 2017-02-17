#include <qubus/qtl/IR/sum_expr.hpp>

#include <qubus/IR/type.hpp>

#include <qubus/util/assert.hpp>

#include <utility>

namespace qubus
{
namespace qtl
{

sum_expr::sum_expr(variable_declaration contraction_index_, std::unique_ptr<expression> body_)
: body_(take_over_child(body_)), contraction_indices_{std::move(contraction_index_)}
{
}

sum_expr::sum_expr(std::vector<variable_declaration> contraction_indices_,
                   std::unique_ptr<expression> body_)
: body_(take_over_child(body_)), contraction_indices_(std::move(contraction_indices_))
{
}

sum_expr::sum_expr(std::vector<variable_declaration> contraction_indices_,
                   variable_declaration alias_, std::unique_ptr<expression> body_)
: body_(take_over_child(body_)), contraction_indices_(std::move(contraction_indices_)),
  alias_(std::move(alias_))
{
}

const expression& sum_expr::body() const
{
    return *body_;
}

const std::vector<variable_declaration>& sum_expr::contraction_indices() const
{
    return contraction_indices_;
}

const boost::optional<variable_declaration>& sum_expr::alias() const
{
    return alias_;
}

sum_expr* sum_expr::clone() const
{
    if (alias_)
    {
        return new sum_expr(contraction_indices_, *alias_, qubus::clone(*body_));
    }
    else
    {
        return new sum_expr(contraction_indices_, qubus::clone(*body_));
    }
}

const expression& sum_expr::child(std::size_t index) const
{
    if (index == 0)
    {
        return *body_;
    }
    else
    {
        throw 0;
    }
}

std::size_t sum_expr::arity() const
{
    return 1;
}

std::unique_ptr<expression>
sum_expr::substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 1)
        throw 0;

    if (alias_)
    {
        return std::make_unique<sum_expr>(contraction_indices_, *alias_,
                                          std::move(new_children[0]));
    }
    else
    {
        return std::make_unique<sum_expr>(contraction_indices_, std::move(new_children[0]));
    }
}

bool operator==(const sum_expr& lhs, const sum_expr& rhs)
{
    return lhs.contraction_indices() == rhs.contraction_indices() && lhs.body() == rhs.body();
}

bool operator!=(const sum_expr& lhs, const sum_expr& rhs)
{
    return !(lhs == rhs);
}

QUBUS_DEFINE_MULTI_METHOD_SPECIALIZATION_WITH_NAME(equal, std::equal_to<sum_expr>(), sum_expr_equal);

}
}