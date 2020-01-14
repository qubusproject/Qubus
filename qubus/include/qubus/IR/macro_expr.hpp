#ifndef QUBUS_MACRO_EXPR_HPP
#define QUBUS_MACRO_EXPR_HPP

#include <hpx/config.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/IR/variable_declaration.hpp>
#include <qubus/util/unused.hpp>

#include <vector>

namespace qubus
{

class macro_expr final : public expression_base<macro_expr>
{
public:
    macro_expr() = default;
    macro_expr(std::vector<std::shared_ptr<const variable_declaration>> params_,
               std::unique_ptr<expression> body_);

    virtual ~macro_expr() = default;

    const std::vector<std::shared_ptr<const variable_declaration>>& params() const;
    const expression& body() const;

    macro_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

private:
    std::vector<std::shared_ptr<const variable_declaration>> params_;
    std::unique_ptr<expression> body_;
};

bool operator==(const macro_expr& lhs, const macro_expr& rhs);
bool operator!=(const macro_expr& lhs, const macro_expr& rhs);

inline std::unique_ptr<macro_expr>
make_macro(std::vector<std::shared_ptr<const variable_declaration>> params,
           std::unique_ptr<expression> body)
{
    return std::make_unique<macro_expr>(std::move(params), std::move(body));
}

std::unique_ptr<expression> expand_macro(const macro_expr& macro,
                                         std::vector<std::unique_ptr<expression>> args);

} // namespace qubus

#endif