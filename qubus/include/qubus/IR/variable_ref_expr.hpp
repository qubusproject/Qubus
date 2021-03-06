#ifndef QUBUS_VARIABLE_REF_EXPR_HPP
#define QUBUS_VARIABLE_REF_EXPR_HPP

#include <hpx/config.hpp>

#include <qubus/IR/access.hpp>
#include <qubus/IR/variable_declaration.hpp>

#include <qubus/util/hash.hpp>
#include <qubus/util/unused.hpp>

#include <memory>
#include <vector>
#include <functional>

namespace qubus
{
    
class variable_ref_expr final : public access_expr_base<variable_ref_expr>
{
public:
    variable_ref_expr() = default;
    explicit variable_ref_expr(variable_declaration declaration_);

    virtual ~variable_ref_expr() = default;
    
    const variable_declaration& declaration() const;

    variable_ref_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
            std::vector<std::unique_ptr<expression>> new_children) const override final;

private:
    variable_declaration declaration_;
    
    mutable annotation_map annotations_;
};

bool operator==(const variable_ref_expr& lhs, const variable_ref_expr& rhs);
bool operator!=(const variable_ref_expr& lhs, const variable_ref_expr& rhs);

inline std::unique_ptr<variable_ref_expr> variable_ref(variable_declaration var)
{
    return std::make_unique<variable_ref_expr>(std::move(var));
}

inline std::unique_ptr<variable_ref_expr> var(variable_declaration var)
{
    return variable_ref(std::move(var));
}

}

#endif