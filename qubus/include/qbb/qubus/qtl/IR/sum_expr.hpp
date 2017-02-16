#ifndef QBB_QUBUS_QTL_SUM_EXPR_HPP
#define QBB_QUBUS_QTL_SUM_EXPR_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/vector.hpp>
#include <qbb/util/hpx/serialization/optional.hpp>

#include <boost/optional.hpp>

#include <vector>

inline namespace qbb
{
namespace qubus
{
namespace qtl
{

class sum_expr : public expression_base<sum_expr>
{
public:
    sum_expr() = default;
    sum_expr(variable_declaration contraction_index_, std::unique_ptr<expression> body_);
    sum_expr(std::vector<variable_declaration> contraction_indices_,
             std::unique_ptr<expression> body_);
    sum_expr(std::vector<variable_declaration> contraction_indices_, variable_declaration alias_,
             std::unique_ptr<expression> body_);

    virtual ~sum_expr() = default;

    const expression& body() const;

    const std::vector<variable_declaration>& contraction_indices() const;
    const boost::optional<variable_declaration>& alias() const;

    sum_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& body_;
        ar& contraction_indices_;
        ar& alias_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(sum_expr);

private:
    std::unique_ptr<expression> body_;
    std::vector<variable_declaration> contraction_indices_;
    boost::optional<variable_declaration> alias_;
};

bool operator==(const sum_expr& lhs, const sum_expr& rhs);
bool operator!=(const sum_expr& lhs, const sum_expr& rhs);

inline std::unique_ptr<sum_expr> sum(variable_declaration contraction_index,
                                     std::unique_ptr<expression> body)
{
    return std::make_unique<sum_expr>(std::move(contraction_index), std::move(body));
}

inline std::unique_ptr<sum_expr> sum(std::vector<variable_declaration> contraction_indices,
                                     std::unique_ptr<expression> body)
{
    return std::make_unique<sum_expr>(std::move(contraction_indices), std::move(body));
}

inline std::unique_ptr<sum_expr> sum(std::vector<variable_declaration> contraction_indices,
                                     variable_declaration alias, std::unique_ptr<expression> body)
{
    return std::make_unique<sum_expr>(std::move(contraction_indices), std::move(alias),
                                      std::move(body));
}
}
}
}

#endif