#include <qbb/qubus/IR/subscription_expr.hpp>

#include <utility>

inline namespace qbb
{
namespace qubus
{

subscription_expr::subscription_expr(std::unique_ptr<access_expr> indexed_expr_, std::vector<std::unique_ptr<expression>> indices_)
: indexed_expr_{take_over_child(indexed_expr_)}, indices_(take_over_children(indices_))
{
}

const access_expr& subscription_expr::indexed_expr() const
{
    return *indexed_expr_;
}

const access_expr& subscription_expr::qualified_access() const
{
    return indexed_expr();
}

subscription_expr* subscription_expr::clone() const
{
    return new subscription_expr(qbb::qubus::clone(*indexed_expr_), qbb::qubus::clone(indices_));
}

const expression& subscription_expr::child(std::size_t index) const
{
    if (index == 0)
    {
        return *indexed_expr_;
    }
    else if (index - 1 < indices_.size())
    {
        return *indices_[index - 1];
    }
    else
    {
        throw 0;
    }
}

std::size_t subscription_expr::arity() const
{
    return indices_.size() + 1;
}

std::unique_ptr<expression> subscription_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() < 1)
        throw 0;

    std::unique_ptr<access_expr> indexed_access(dynamic_cast<access_expr*>(new_children[0].release()));

    if (!indexed_access)
        throw 0;

    std::vector<std::unique_ptr<expression>> indices;
    indices.reserve(new_children.size() - 1);

    auto number_of_indices = new_children.size();
    for (std::size_t i = 1; i < number_of_indices; ++i)
    {
        indices.push_back(std::move(new_children[i]));
    }

    return std::make_unique<subscription_expr>(std::move(indexed_access), std::move(indices));
}

bool operator==(const subscription_expr& lhs, const subscription_expr& rhs)
{
    return lhs.indexed_expr() == rhs.indexed_expr() && lhs.indices() == rhs.indices();
}

bool operator!=(const subscription_expr& lhs, const subscription_expr& rhs)
{
    return !(lhs == rhs);
}

}
}