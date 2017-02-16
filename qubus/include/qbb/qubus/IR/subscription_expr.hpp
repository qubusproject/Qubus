#ifndef QBB_QUBUS_SUBSCRIPTION_EXPR_HPP
#define QBB_QUBUS_SUBSCRIPTION_EXPR_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/access_qualifier.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/vector.hpp>

#include <boost/range/adaptor/indirected.hpp>

#include <vector>

inline namespace qbb
{
namespace qubus
{

class subscription_expr : public access_qualifier_base<subscription_expr>
{
public:
    subscription_expr() = default;
    subscription_expr(std::unique_ptr<access_expr> indexed_expr_,
                      std::vector<std::unique_ptr<expression>> indices_);

    virtual ~subscription_expr() = default;

    const access_expr& indexed_expr() const;

    auto indices() const
    {
        return indices_ | boost::adaptors::indirected;
    }

    const access_expr& qualified_access() const override final;

    subscription_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& indexed_expr_;
        ar& indices_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(subscription_expr);

private:
    std::unique_ptr<access_expr> indexed_expr_;
    std::vector<std::unique_ptr<expression>> indices_;
};

bool operator==(const subscription_expr& lhs, const subscription_expr& rhs);
bool operator!=(const subscription_expr& lhs, const subscription_expr& rhs);

inline std::unique_ptr<subscription_expr>
subscription(std::unique_ptr<access_expr> indexed_expr,
             std::vector<std::unique_ptr<expression>> indices)
{
    return std::make_unique<subscription_expr>(std::move(indexed_expr), std::move(indices));
}

inline auto
subscription(std::unique_ptr<access_expr> indexed_expr, std::unique_ptr<expression> index)
{
    std::vector<std::unique_ptr<expression>> indices;
    indices.reserve(1);
    indices.push_back(std::move(index));

    return subscription(std::move(indexed_expr), std::move(indices));
}

}
}

#endif