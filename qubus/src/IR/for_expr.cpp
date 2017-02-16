#include <qbb/qubus/IR/for_expr.hpp>

#include <qbb/qubus/IR/literal_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

inline namespace qbb
{
namespace qubus
{

for_expr::for_expr(variable_declaration loop_index_, std::unique_ptr<expression> lower_bound_,
                   std::unique_ptr<expression> upper_bound_, std::unique_ptr<expression> body_)
: for_expr(std::move(loop_index_), std::move(lower_bound_), std::move(upper_bound_),
           integer_literal(1), std::move(body_))
{
}

for_expr::for_expr(variable_declaration loop_index_, std::unique_ptr<expression> lower_bound_,
                   std::unique_ptr<expression> upper_bound_, std::unique_ptr<expression> increment_,
                   std::unique_ptr<expression> body_)
: for_expr(execution_order::sequential, std::move(loop_index_), std::move(lower_bound_),
           std::move(upper_bound_), std::move(increment_), std::move(body_))

{
}

for_expr::for_expr(execution_order order_, variable_declaration loop_index_,
                   std::unique_ptr<expression> lower_bound_,
                   std::unique_ptr<expression> upper_bound_, std::unique_ptr<expression> body_)
: for_expr(std::move(order_), std::move(loop_index_), std::move(lower_bound_),
           std::move(upper_bound_), integer_literal(1), std::move(body_))
{
}

for_expr::for_expr(execution_order order_, variable_declaration loop_index_,
                   std::unique_ptr<expression> lower_bound_,
                   std::unique_ptr<expression> upper_bound_, std::unique_ptr<expression> increment_,
                   std::unique_ptr<expression> body_)
: order_(std::move(order_)), loop_index_(std::move(loop_index_)),
  lower_bound_(take_over_child(lower_bound_)), upper_bound_(take_over_child(upper_bound_)),
  increment_(take_over_child(increment_)), body_(take_over_child(body_))
{
}

execution_order for_expr::order() const
{
    return order_;
}

const expression& for_expr::body() const
{
    return *body_;
}

const variable_declaration& for_expr::loop_index() const
{
    return loop_index_;
}

const expression& for_expr::lower_bound() const
{
    return *lower_bound_;
}

const expression& for_expr::upper_bound() const
{
    return *upper_bound_;
}

const expression& for_expr::increment() const
{
    return *increment_;
}

for_expr* for_expr::clone() const
{
    return new for_expr(order_, loop_index_, qbb::qubus::clone(*lower_bound_),
                        qbb::qubus::clone(*upper_bound_), qbb::qubus::clone(*increment_),
                        qbb::qubus::clone(*body_));
}

const expression& for_expr::child(std::size_t index) const
{
    if (index == 0)
    {
        return *lower_bound_;
    }
    else if (index == 1)
    {
        return *upper_bound_;
    }
    else if (index == 2)
    {
        return *increment_;
    }
    else if (index == 3)
    {
        return *body_;
    }
    else
    {
        throw 0;
    }
}

std::size_t for_expr::arity() const
{
    return 4;
}

std::unique_ptr<expression>
for_expr::substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 4)
        throw 0;

    return std::make_unique<for_expr>(order_, loop_index_, std::move(new_children[0]),
                                      std::move(new_children[1]), std::move(new_children[2]),
                                      std::move(new_children[3]));
}

bool operator==(const for_expr& lhs, const for_expr& rhs)
{
    return lhs.order() == rhs.order() && lhs.loop_index() == rhs.loop_index() &&
           lhs.lower_bound() == rhs.lower_bound() && lhs.upper_bound() == rhs.upper_bound() &&
           lhs.increment() == rhs.increment() && lhs.body() == rhs.body();
}

bool operator!=(const for_expr& lhs, const for_expr& rhs)
{
    return !(lhs == rhs);
}
}
}