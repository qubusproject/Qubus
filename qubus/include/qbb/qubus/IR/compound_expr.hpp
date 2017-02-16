#ifndef QUBUS_COMPOUND_EXPR_HPP
#define QUBUS_COMPOUND_EXPR_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/execution_order.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/vector.hpp>

#include <boost/range/adaptor/indirected.hpp>

#include <vector>

namespace qubus
{

class compound_expr : public expression_base<compound_expr>
{
public:
    compound_expr() = default;
    explicit compound_expr(std::vector<std::unique_ptr<expression>> body_);
    compound_expr(execution_order order_, std::vector<std::unique_ptr<expression>> body_);

    virtual ~compound_expr() = default;

    execution_order order() const;
    
    auto body() const
    {
        return body_ | boost::adaptors::indirected;
    }

    compound_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
            std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & order_;
        ar & body_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(compound_expr);
private:
    execution_order order_;
    std::vector<std::unique_ptr<expression>> body_;
};

bool operator==(const compound_expr& lhs, const compound_expr& rhs);
bool operator!=(const compound_expr& lhs, const compound_expr& rhs);

inline std::unique_ptr<compound_expr> sequenced_tasks(std::vector<std::unique_ptr<expression>> tasks)
{
    return std::make_unique<compound_expr>(std::move(tasks));
}

inline std::unique_ptr<compound_expr> sequenced_tasks(std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs)
{
    std::vector<std::unique_ptr<expression>> expressions;
    expressions.reserve(2);
    expressions.push_back(std::move(lhs));
    expressions.push_back(std::move(rhs));

    return sequenced_tasks(std::move(expressions));
}

inline std::unique_ptr<compound_expr> unordered_tasks(std::vector<std::unique_ptr<expression>> tasks)
{
    return std::make_unique<compound_expr>(execution_order::unordered, std::move(tasks));
}

inline std::unique_ptr<compound_expr> unordered_tasks(std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs)
{
    std::vector<std::unique_ptr<expression>> expressions;
    expressions.reserve(2);
    expressions.push_back(std::move(lhs));
    expressions.push_back(std::move(rhs));

    return unordered_tasks(std::move(expressions));
}

}

#endif