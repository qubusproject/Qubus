#ifndef QBB_QUBUS_FOR_ALL_EXPR_HPP
#define QBB_QUBUS_FOR_ALL_EXPR_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>

#include <boost/optional.hpp>

#include <hpx/runtime/serialization/vector.hpp>
#include <qbb/util/hpx/serialization/optional.hpp>

#include <qbb/util/unused.hpp>
#include <vector>

namespace qbb
{
namespace qubus
{

class for_all_expr : public expression_base<for_all_expr>
{
public:
    for_all_expr() = default;
    for_all_expr(variable_declaration loop_index_, std::unique_ptr<expression> body_);
    for_all_expr(std::vector<variable_declaration> loop_indices_,
                 std::unique_ptr<expression> body_);
    for_all_expr(std::vector<variable_declaration> loop_indices_, variable_declaration alias_,
                 std::unique_ptr<expression> body_);

    virtual ~for_all_expr() = default;

    const expression& body() const;

    const std::vector<variable_declaration>& loop_indices() const;
    const boost::optional<variable_declaration>& alias() const;

    for_all_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& loop_indices_;
        ar& alias_;
        ar& body_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(for_all_expr);

private:
    std::vector<variable_declaration> loop_indices_;
    boost::optional<variable_declaration> alias_;
    std::unique_ptr<expression> body_;
};

bool operator==(const for_all_expr& lhs, const for_all_expr& rhs);
bool operator!=(const for_all_expr& lhs, const for_all_expr& rhs);

inline std::unique_ptr<for_all_expr> for_all(variable_declaration loop_index,
                                             std::unique_ptr<expression> body)
{
    return std::make_unique<for_all_expr>(std::move(loop_index), std::move(body));
}

inline std::unique_ptr<for_all_expr> for_all(std::vector<variable_declaration> loop_indices,
                                             std::unique_ptr<expression> body)
{
    return std::make_unique<for_all_expr>(std::move(loop_indices), std::move(body));
}

inline std::unique_ptr<for_all_expr> for_all(std::vector<variable_declaration> loop_indices,
                                             variable_declaration alias,
                                             std::unique_ptr<expression> body)
{
    return std::make_unique<for_all_expr>(std::move(loop_indices), std::move(alias),
                                          std::move(body));
}
}
}

#endif