#ifndef QBB_QUBUS_FOREIGN_CALL_EXPR_HPP
#define QBB_QUBUS_FOREIGN_CALL_EXPR_HPP

#include <qbb/qubus/foreign_computelet.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/string.hpp>
#include <hpx/runtime/serialization/vector.hpp>

#include <boost/range/adaptor/indirected.hpp>

#include <string>
#include <vector>

inline namespace qbb
{
namespace qubus
{

class foreign_call_expr : public expression_base<foreign_call_expr>
{
public:
    foreign_call_expr() = default;
    foreign_call_expr(foreign_computelet computelet_, std::vector<std::unique_ptr<expression>> args_);

    virtual ~foreign_call_expr() = default;

    type result_type() const;
    const foreign_computelet& computelet() const;

    auto args() const
    {
        return args_ | boost::adaptors::indirected;
    }

    foreign_call_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
            std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & computelet_;
        ar & args_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(foreign_call_expr);
private:
    foreign_computelet computelet_;
    std::vector<std::unique_ptr<expression>> args_;
};

bool operator==(const foreign_call_expr& lhs, const foreign_call_expr& rhs);
bool operator!=(const foreign_call_expr& lhs, const foreign_call_expr& rhs);

inline std::unique_ptr<foreign_call_expr> foreign_call(foreign_computelet computelet, std::vector<std::unique_ptr<expression>> args)
{
    return std::make_unique<foreign_call_expr>(std::move(computelet), std::move(args));
}

}
}

#endif
