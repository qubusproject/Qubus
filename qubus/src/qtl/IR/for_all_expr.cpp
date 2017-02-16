#include <qubus/qtl/IR/for_all_expr.hpp>

#include <qubus/util/assert.hpp>

#include <utility>

namespace qubus
{
namespace qtl
{

for_all_expr::for_all_expr(variable_declaration loop_index_, std::unique_ptr<expression> body_)
: loop_indices_{std::move(loop_index_)}, body_(take_over_child(body_))
{
}

for_all_expr::for_all_expr(std::vector<variable_declaration> loop_indices_,
                           std::unique_ptr<expression> body_)
: loop_indices_(std::move(loop_indices_)), body_(take_over_child(body_))
{
    QUBUS_ASSERT(this->loop_indices_.size() > 0,
               "A for all loops needs to declare at least one index.");
}

for_all_expr::for_all_expr(std::vector<variable_declaration> loop_indices_,
                           variable_declaration alias_, std::unique_ptr<expression> body_)
: loop_indices_(std::move(loop_indices_)), alias_(std::move(alias_)), body_(take_over_child(body_))
{
    QUBUS_ASSERT(this->loop_indices_.size() > 0,
               "A for all loops needs to declare at least one index.");
}

const expression& for_all_expr::body() const
{
    return *body_;
}

const std::vector<variable_declaration>& for_all_expr::loop_indices() const
{
    return loop_indices_;
}

const boost::optional<variable_declaration>& for_all_expr::alias() const
{
    return alias_;
}

for_all_expr* for_all_expr::clone() const
{
    if (alias_)
    {
        return new for_all_expr(loop_indices_, *alias_, qubus::clone(*body_));
    }
    else
    {
        return new for_all_expr(loop_indices_, qubus::clone(*body_));
    }
}

const expression& for_all_expr::child(std::size_t index) const
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

std::size_t for_all_expr::arity() const
{
    return 1;
}

std::unique_ptr<expression>
for_all_expr::substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 1)
        throw 0;

    if (alias_)
    {
        return std::make_unique<for_all_expr>(loop_indices_, *alias_, std::move(new_children[0]));
    }
    else
    {
        return std::make_unique<for_all_expr>(loop_indices_, std::move(new_children[0]));
    }
}

bool operator==(const for_all_expr& lhs, const for_all_expr& rhs)
{
    return lhs.loop_indices() == rhs.loop_indices() && lhs.body() == rhs.body();
}

bool operator!=(const for_all_expr& lhs, const for_all_expr& rhs)
{
    return !(lhs == rhs);
}

QUBUS_DEFINE_MULTI_METHOD_SPECIALIZATION_WITH_NAME(equal, std::equal_to<for_all_expr>(), for_all_expr_equal);

}
}