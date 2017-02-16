#ifndef QUBUS_FOR_EXPR_HPP
#define QUBUS_FOR_EXPR_HPP

#include <hpx/config.hpp>

#include <qubus/IR/execution_order.hpp>
#include <qubus/IR/expression.hpp>
#include <qubus/IR/expression_traits.hpp>
#include <qubus/IR/variable_declaration.hpp>
#include <qubus/util/unused.hpp>

#include <vector>

namespace qubus
{

class for_expr : public expression_base<for_expr>
{
public:
    for_expr() = default;
    for_expr(variable_declaration loop_index_, std::unique_ptr<expression> lower_bound_,
             std::unique_ptr<expression> upper_bound_, std::unique_ptr<expression> body_);
    for_expr(variable_declaration loop_index_, std::unique_ptr<expression> lower_bound_,
             std::unique_ptr<expression> upper_bound_, std::unique_ptr<expression> increment_,
             std::unique_ptr<expression> body_);
    for_expr(execution_order order_, variable_declaration loop_index_,
             std::unique_ptr<expression> lower_bound_, std::unique_ptr<expression> upper_bound_,
             std::unique_ptr<expression> body_);
    for_expr(execution_order order_, variable_declaration loop_index_,
             std::unique_ptr<expression> lower_bound_, std::unique_ptr<expression> upper_bound_,
             std::unique_ptr<expression> increment_, std::unique_ptr<expression> body_);

    virtual ~for_expr() = default;

    execution_order order() const;

    const expression& body() const;

    const variable_declaration& loop_index() const;

    const expression& lower_bound() const;
    const expression& upper_bound() const;
    const expression& increment() const;

    for_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& order_;
        ar& loop_index_;
        ar& lower_bound_;
        ar& upper_bound_;
        ar& increment_;
        ar& body_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(for_expr);

private:
    execution_order order_;
    variable_declaration loop_index_;
    std::unique_ptr<expression> lower_bound_;
    std::unique_ptr<expression> upper_bound_;
    std::unique_ptr<expression> increment_;
    std::unique_ptr<expression> body_;
};

bool operator==(const for_expr& lhs, const for_expr& rhs);
bool operator!=(const for_expr& lhs, const for_expr& rhs);

inline std::unique_ptr<for_expr> for_(variable_declaration loop_index,
                                      std::unique_ptr<expression> lower_bound,
                                      std::unique_ptr<expression> upper_bound,
                                      std::unique_ptr<expression> body)
{
    return std::make_unique<for_expr>(std::move(loop_index), std::move(lower_bound),
                                      std::move(upper_bound), std::move(body));
}

inline std::unique_ptr<for_expr> for_(variable_declaration loop_index,
                                      std::unique_ptr<expression> lower_bound,
                                      std::unique_ptr<expression> upper_bound,
                                      std::unique_ptr<expression> increment,
                                      std::unique_ptr<expression> body)
{
    return std::make_unique<for_expr>(std::move(loop_index), std::move(lower_bound),
                                      std::move(upper_bound), std::move(increment),
                                      std::move(body));
}

inline std::unique_ptr<for_expr> unordered_for(variable_declaration loop_index,
                                               std::unique_ptr<expression> lower_bound,
                                               std::unique_ptr<expression> upper_bound,
                                               std::unique_ptr<expression> body)
{
    return std::make_unique<for_expr>(execution_order::unordered, std::move(loop_index),
                                      std::move(lower_bound), std::move(upper_bound),
                                      std::move(body));
}

inline std::unique_ptr<for_expr> unordered_for(variable_declaration loop_index,
                                               std::unique_ptr<expression> lower_bound,
                                               std::unique_ptr<expression> upper_bound,
                                               std::unique_ptr<expression> increment,
                                               std::unique_ptr<expression> body)
{
    return std::make_unique<for_expr>(execution_order::unordered, std::move(loop_index),
                                      std::move(lower_bound), std::move(upper_bound),
                                      std::move(increment), std::move(body));
}
}

#endif