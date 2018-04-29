#include <qubus/IR/integer_range_expr.hpp>

#include <qubus/IR/literal_expr.hpp>

#include <utility>

namespace qubus
{

integer_range_expr::integer_range_expr(std::unique_ptr<expression> lower_bound_,
                                       std::unique_ptr<expression> upper_bound_)
: lower_bound_(std::move(lower_bound_)), upper_bound_(std::move(upper_bound_)), stride_(lit(1l))
{
}

integer_range_expr::integer_range_expr(std::unique_ptr<expression> lower_bound_,
                                       std::unique_ptr<expression> upper_bound_,
                                       std::unique_ptr<expression> stride_)
: lower_bound_(std::move(lower_bound_)),
  upper_bound_(std::move(upper_bound_)),
  stride_(std::move(stride_))
{
}

const expression& integer_range_expr::lower_bound() const
{
    return *lower_bound_;
}

const expression& integer_range_expr::upper_bound() const
{
    return *upper_bound_;
}

const expression& integer_range_expr::stride() const
{
    return *stride_;
}

integer_range_expr* integer_range_expr::clone() const
{
    return new integer_range_expr(qubus::clone(lower_bound()), qubus::clone(upper_bound()), qubus::clone(stride()));
}

const expression& integer_range_expr::child(std::size_t index) const
{
    if (index == 0)
        return lower_bound();

    if (index == 1)
        return upper_bound();

    if (index == 2)
        return stride();

    throw 0;
}

std::size_t integer_range_expr::arity() const
{
    return 3;
}

std::unique_ptr<expression> integer_range_expr::substitute_subexpressions(
    std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 3)
        throw 0;

    return std::make_unique<integer_range_expr>(
        std::move(new_children[0]), std::move(new_children[1]), std::move(new_children[2]));
}

bool operator==(const integer_range_expr& lhs, const integer_range_expr& rhs)
{
    return lhs.lower_bound() == rhs.lower_bound() && lhs.upper_bound() == rhs.upper_bound() && lhs.stride() == rhs.stride();
}

bool operator!=(const integer_range_expr& lhs, const integer_range_expr& rhs)
{
    return !(lhs == rhs);
}

} // namespace qubus
