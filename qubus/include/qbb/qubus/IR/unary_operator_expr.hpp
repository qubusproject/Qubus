#ifndef QBB_QUBUS_UNARY_OPERATOR_EXPR_HPP
#define QBB_QUBUS_UNARY_OPERATOR_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

enum class unary_op_tag
{ nop,
  plus,
  negate,
  logical_not };

class unary_operator_expr : public expression_base<unary_operator_expr>
{
public:
    unary_operator_expr();
    unary_operator_expr(unary_op_tag tag_, std::unique_ptr<expression> arg_);

    virtual ~unary_operator_expr() = default;

    unary_op_tag tag() const;

    const expression& arg() const;

    unary_operator_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
            std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & tag_;
        ar & arg_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(unary_operator_expr);
private:
    unary_op_tag tag_;
    std::unique_ptr<expression> arg_;
};

bool operator==(const unary_operator_expr& lhs, const unary_operator_expr& rhs);
bool operator!=(const unary_operator_expr& lhs, const unary_operator_expr& rhs);

inline std::unique_ptr<unary_operator_expr> unary_operator(unary_op_tag tag, std::unique_ptr<expression> arg)
{
    return std::make_unique<unary_operator_expr>(tag, std::move(arg));
}

inline std::unique_ptr<unary_operator_expr> nop(std::unique_ptr<expression> arg)
{
    return unary_operator(unary_op_tag::nop, std::move(arg));
}

inline std::unique_ptr<unary_operator_expr> operator+(std::unique_ptr<expression> arg)
{
    return unary_operator(unary_op_tag::plus, std::move(arg));
}

inline std::unique_ptr<unary_operator_expr> operator-(std::unique_ptr<expression> arg)
{
    return unary_operator(unary_op_tag::negate, std::move(arg));
}

inline std::unique_ptr<unary_operator_expr> logical_not(std::unique_ptr<expression> arg)
{
    return unary_operator(unary_op_tag::logical_not, std::move(arg));
}

}
}

#endif