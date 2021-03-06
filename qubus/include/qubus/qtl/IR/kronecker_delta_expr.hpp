#ifndef QUBUS_QTL_KRONECKER_DELTA_HPP
#define QUBUS_QTL_KRONECKER_DELTA_HPP

#include <qubus/IR/expression.hpp>

#include <qubus/util/integers.hpp>
#include <qubus/util/unused.hpp>

#include <vector>

namespace qubus
{
namespace qtl
{

class kronecker_delta_expr final : public expression_base<kronecker_delta_expr>
{
public:
    kronecker_delta_expr() = default;
    kronecker_delta_expr(util::index_t extent_, std::unique_ptr<expression> first_index_,
                         std::unique_ptr<expression> second_index_);

    virtual ~kronecker_delta_expr() = default;

    util::index_t extent() const;
    const expression& first_index() const;
    const expression& second_index() const;

    kronecker_delta_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

private:
    util::index_t extent_;
    std::unique_ptr<expression> first_index_;
    std::unique_ptr<expression> second_index_;
};

bool operator==(const kronecker_delta_expr& lhs, const kronecker_delta_expr& rhs);
bool operator!=(const kronecker_delta_expr& lhs, const kronecker_delta_expr& rhs);

inline std::unique_ptr<kronecker_delta_expr>
kronecker_delta(util::index_t extent, std::unique_ptr<expression> first_index,
                std::unique_ptr<expression> second_index)
{
    return std::make_unique<kronecker_delta_expr>(std::move(extent), std::move(first_index),
                                                  std::move(second_index));
}
}
}

#endif
