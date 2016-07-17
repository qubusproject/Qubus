#ifndef QBB_QUBUS_CONSTRUCT_EXPR_HPP
#define QBB_QUBUS_CONSTRUCT_EXPR_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/type.hpp>
#include <qbb/util/unused.hpp>

#include <boost/range/adaptor/indirected.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class construct_expr : public expression_base<construct_expr>
{
public:
    construct_expr() = default;
    construct_expr(type result_type_, std::vector<std::unique_ptr<expression>> parameters_);

    virtual ~construct_expr() = default;

    const type& result_type() const;

    auto parameters() const
    {
        return parameters_ | boost::adaptors::indirected;
    }

    construct_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
            std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & result_type_;
        ar & parameters_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(construct_expr);
private:
    type result_type_;
    std::vector<std::unique_ptr<expression>> parameters_;
};

bool operator==(const construct_expr& lhs, const construct_expr& rhs);
bool operator!=(const construct_expr& lhs, const construct_expr& rhs);

inline std::unique_ptr<construct_expr> construct(type result_type, std::vector<std::unique_ptr<expression>> parameters)
{
    return std::make_unique<construct_expr>(std::move(result_type), std::move(parameters));
}

}
}

#endif
