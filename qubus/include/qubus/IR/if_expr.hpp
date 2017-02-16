#ifndef QUBUS_IF_EXPR_HPP
#define QUBUS_IF_EXPR_HPP

#include <hpx/config.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/IR/variable_declaration.hpp>
#include <qubus/util/unused.hpp>

#include <qubus/util/hpx/serialization/optional.hpp>

#include <qubus/util/optional_ref.hpp>

#include <boost/optional.hpp>

namespace qubus
{

class if_expr : public expression_base<if_expr>
{
public:
    if_expr() = default;
    if_expr(std::unique_ptr<expression> condition_, std::unique_ptr<expression> then_branch_);
    if_expr(std::unique_ptr<expression> condition_, std::unique_ptr<expression> then_branch_,
            std::unique_ptr<expression> else_branch_);

    virtual ~if_expr() = default;

    const expression& condition() const;

    const expression& then_branch() const;
    util::optional_ref<const expression> else_branch() const;

    if_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void save(Archive& ar, unsigned QUBUS_UNUSED(version)) const
    {
        ar << condition_;
        ar << then_branch_;
        ar << static_cast<bool>(else_branch_);
        ar << *else_branch_;
    }

    template <typename Archive>
    void load(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar >> condition_;
        ar >> then_branch_;

        bool has_else_branch;

        ar >> has_else_branch;

        if (has_else_branch)
        {
            std::unique_ptr<expression> else_branch;

            ar >> else_branch;

            else_branch_ = std::move(else_branch);
        }
    }

    HPX_SERIALIZATION_POLYMORPHIC_SPLITTED(if_expr);

private:
    std::unique_ptr<expression> condition_;
    std::unique_ptr<expression> then_branch_;
    boost::optional<std::unique_ptr<expression>> else_branch_;
};

bool operator==(const if_expr& lhs, const if_expr& rhs);
bool operator!=(const if_expr& lhs, const if_expr& rhs);

inline std::unique_ptr<if_expr> if_(std::unique_ptr<expression> condition,
                                    std::unique_ptr<expression> then_branch)
{
    return std::make_unique<if_expr>(std::move(condition), std::move(then_branch));
}

inline std::unique_ptr<if_expr> if_(std::unique_ptr<expression> condition,
                                    std::unique_ptr<expression> then_branch,
                                    std::unique_ptr<expression> else_branch)
{
    return std::make_unique<if_expr>(std::move(condition), std::move(then_branch),
                                     std::move(else_branch));
}
}

#endif
