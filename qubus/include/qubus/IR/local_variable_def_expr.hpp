#ifndef QUBUS_LOCAL_VARIABLE_DEF_EXPR_HPP
#define QUBUS_LOCAL_VARIABLE_DEF_EXPR_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/expression_traits.hpp>
#include <qubus/IR/variable_declaration.hpp>
#include <qubus/util/unused.hpp>

#include <vector>

namespace qubus
{

class local_variable_def_expr : public expression_base<local_variable_def_expr>
{
public:
    local_variable_def_expr() = default;
    local_variable_def_expr(variable_declaration decl_, std::unique_ptr<expression> initializer_);

    virtual ~local_variable_def_expr() = default;

    const variable_declaration& decl() const;
    const expression& initializer() const;

    local_variable_def_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& decl_;
        ar& initializer_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(local_variable_def_expr);

private:
    variable_declaration decl_;
    std::unique_ptr<expression> initializer_;
};

bool operator==(const local_variable_def_expr& lhs, const local_variable_def_expr& rhs);
bool operator!=(const local_variable_def_expr& lhs, const local_variable_def_expr& rhs);

inline std::unique_ptr<local_variable_def_expr>
local_variable_def(variable_declaration decl, std::unique_ptr<expression> initializer)
{
    return std::make_unique<local_variable_def_expr>(std::move(decl), std::move(initializer));
}
}

#endif