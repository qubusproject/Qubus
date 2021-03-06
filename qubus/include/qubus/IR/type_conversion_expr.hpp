#ifndef QUBUS_TYPE_CONVERSION_EXPR_HPP
#define QUBUS_TYPE_CONVERSION_EXPR_HPP

#include <hpx/config.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/IR/type.hpp>
#include <qubus/util/unused.hpp>

#include <vector>

namespace qubus
{

class type_conversion_expr final : public expression_base<type_conversion_expr>
{
public:
    type_conversion_expr() = default;
    type_conversion_expr(type target_type_, std::unique_ptr<expression> arg_);

    virtual ~type_conversion_expr() = default;

    type target_type() const;

    const expression& arg() const;

    type_conversion_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
            std::vector<std::unique_ptr<expression>> new_children) const override final;

private:
    type target_type_;
    std::unique_ptr<expression> arg_;
};

bool operator==(const type_conversion_expr& lhs, const type_conversion_expr& rhs);
bool operator!=(const type_conversion_expr& lhs, const type_conversion_expr& rhs);

inline std::unique_ptr<type_conversion_expr> type_conversion(type target_type, std::unique_ptr<expression> arg)
{
    return std::make_unique<type_conversion_expr>(std::move(target_type), std::move(arg));
}

}

#endif