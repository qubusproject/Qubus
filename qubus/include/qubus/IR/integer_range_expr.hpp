#ifndef QUBUS_INTEGER_RANGE_EXPR_HPP
#define QUBUS_INTEGER_RANGE_EXPR_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/util/unused.hpp>

#include <vector>

namespace qubus
{

class integer_range_expr final : public expression_base<integer_range_expr>
{
public:
    integer_range_expr(std::unique_ptr<expression> lower_bound_,
                       std::unique_ptr<expression> upper_bound_);
    integer_range_expr(std::unique_ptr<expression> lower_bound_,
                       std::unique_ptr<expression> upper_bound_,
                       std::unique_ptr<expression> stride_);

    const expression& lower_bound() const;
    const expression& upper_bound() const;
    const expression& stride() const;

    integer_range_expr* clone() const override;

    const expression& child(std::size_t index) const override;

    std::size_t arity() const override;

    std::unique_ptr<expression>
    substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const override;

private:
    std::unique_ptr<expression> lower_bound_;
    std::unique_ptr<expression> upper_bound_;
    std::unique_ptr<expression> stride_;
};

bool operator==(const integer_range_expr& lhs, const integer_range_expr& rhs);
bool operator!=(const integer_range_expr& lhs, const integer_range_expr& rhs);

inline std::unique_ptr<integer_range_expr> range(std::unique_ptr<expression> lower_bound,
                                                 std::unique_ptr<expression> upper_bound)
{
    return std::make_unique<integer_range_expr>(std::move(lower_bound), std::move(upper_bound));
}

inline std::unique_ptr<integer_range_expr> range(std::unique_ptr<expression> lower_bound,
                                                 std::unique_ptr<expression> upper_bound,
                                                 std::unique_ptr<expression> stride)
{
    return std::make_unique<integer_range_expr>(std::move(lower_bound), std::move(upper_bound),
                                                std::move(stride));
}

} // namespace qubus

#endif
