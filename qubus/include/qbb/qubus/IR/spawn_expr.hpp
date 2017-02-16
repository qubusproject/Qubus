#ifndef QUBUS_SPAWN_EXPR_HPP
#define QUBUS_SPAWN_EXPR_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>
#include <qbb/util/unused.hpp>

#include <boost/range/adaptor/indirected.hpp>

#include <memory>
#include <vector>

namespace qubus
{

class spawn_expr : public expression_base<spawn_expr>
{
public:
    spawn_expr() = default;
    spawn_expr(function_declaration spawned_plan_,
               std::vector<std::unique_ptr<expression>> arguments_);

    virtual ~spawn_expr() = default;

    const function_declaration& spawned_plan() const;
    auto arguments() const
    {
        return arguments_ | boost::adaptors::indirected;
    }

    spawn_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& spawned_plan_;
        ar& arguments_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(spawn_expr);

private:
    function_declaration spawned_plan_;
    std::vector<std::unique_ptr<expression>> arguments_;
};

bool operator==(const spawn_expr& lhs, const spawn_expr& rhs);
bool operator!=(const spawn_expr& lhs, const spawn_expr& rhs);

inline std::unique_ptr<spawn_expr> spawn(function_declaration spawned_plan,
                                         std::vector<std::unique_ptr<expression>> arguments)
{
    return std::make_unique<spawn_expr>(std::move(spawned_plan), std::move(arguments));
}
}

#endif