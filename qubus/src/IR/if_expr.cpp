#include <qbb/qubus/IR/if_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

if_expr::if_expr(std::unique_ptr<expression> condition_, std::unique_ptr<expression> then_branch_)
: condition_(std::move(condition_)), then_branch_(std::move(then_branch_))
{
}

if_expr::if_expr(std::unique_ptr<expression> condition_, std::unique_ptr<expression> then_branch_,
                 std::unique_ptr<expression> else_branch_)
: condition_(std::move(condition_)), then_branch_(std::move(then_branch_)),
  else_branch_(std::move(else_branch_))
{
}

const expression& if_expr::condition() const
{
    return *condition_;
}

const expression& if_expr::then_branch() const
{
    return *then_branch_;
}

util::optional_ref<const expression> if_expr::else_branch() const
{
    if (else_branch_)
    {
        return **else_branch_;
    }
    else
    {
        return {};
    }
}

if_expr* if_expr::clone() const
{
    if (else_branch_)
    {
        return new if_expr(qbb::qubus::clone(*condition_), qbb::qubus::clone(*then_branch_),
                           qbb::qubus::clone(**else_branch_));
    }
    else
    {
        return new if_expr(qbb::qubus::clone(*condition_), qbb::qubus::clone(*then_branch_));
    }
}

const expression& if_expr::child(std::size_t index) const
{
    if (index == 0)
    {
        return *condition_;
    }
    else if (index == 1)
    {
        return *then_branch_;
    }
    else if (index == 2)
    {
        if (else_branch_)
        {
            return **else_branch_;
        }
        else
        {
            throw 0;
        }
    }
    else
    {
        throw 0;
    }
}

std::size_t if_expr::arity() const
{
    if (else_branch_)
    {
        return 3;
    }
    else
    {
        return 2;
    }
}

std::unique_ptr<expression>
if_expr::substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const
{
    if (!(new_children.size() == 2 || new_children.size() == 3))
        throw 0;

    if (new_children.size() == 3)
    {
        return std::make_unique<if_expr>(std::move(new_children[0]), std::move(new_children[1]),
                                         std::move(new_children[2]));
    }
    else
    {
        return std::make_unique<if_expr>(std::move(new_children[0]), std::move(new_children[1]));
    }
}

bool operator==(const if_expr& lhs, const if_expr& rhs)
{
    return lhs.condition() == rhs.condition() && lhs.then_branch() == rhs.then_branch() &&
           lhs.else_branch() == rhs.else_branch();
}

bool operator!=(const if_expr& lhs, const if_expr& rhs)
{
    return !(lhs == rhs);
}
}
}
