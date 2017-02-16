#ifndef QBB_QUBUS_BINARY_OPERATOR_EXPR_HPP
#define QBB_QUBUS_BINARY_OPERATOR_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

inline namespace qbb
{
namespace qubus
{

enum class binary_op_tag
{
    nop,
    plus,
    minus,
    multiplies,
    divides,
    modulus,
    div_floor,
    assign,
    plus_assign,
    equal_to,
    not_equal_to,
    less,
    greater,
    less_equal,
    greater_equal,
    logical_and,
    logical_or
};

class binary_operator_expr : public expression_base<binary_operator_expr>
{
public:
    binary_operator_expr();
    binary_operator_expr(binary_op_tag tag_, std::unique_ptr<expression> left_,
                         std::unique_ptr<expression> right_);

    virtual ~binary_operator_expr() = default;

    binary_op_tag tag() const;

    const expression& left() const;
    const expression& right() const;

    binary_operator_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& tag_;
        ar& left_;
        ar& right_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(binary_operator_expr);

private:
    binary_op_tag tag_;
    std::unique_ptr<expression> left_;
    std::unique_ptr<expression> right_;
};

bool operator==(const binary_operator_expr& lhs, const binary_operator_expr& rhs);
bool operator!=(const binary_operator_expr& lhs, const binary_operator_expr& rhs);

inline std::unique_ptr<binary_operator_expr> binary_operator(binary_op_tag tag,
                                                             std::unique_ptr<expression> left,
                                                             std::unique_ptr<expression> right)
{
    return std::make_unique<binary_operator_expr>(tag, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> operator+(std::unique_ptr<expression> left,
                                                       std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::plus, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> operator-(std::unique_ptr<expression> left,
                                                       std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::minus, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> operator*(std::unique_ptr<expression> left,
                                                       std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::multiplies, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> operator/(std::unique_ptr<expression> left,
                                                       std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::divides, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> operator%(std::unique_ptr<expression> left,
                                                       std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::modulus, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> div_floor(std::unique_ptr<expression> left,
                                                       std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::div_floor, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> assign(std::unique_ptr<expression> left,
                                                    std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::assign, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> plus_assign(std::unique_ptr<expression> left,
                                                         std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::plus_assign, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> equal_to(std::unique_ptr<expression> left,
                                                      std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::equal_to, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> not_equal_to(std::unique_ptr<expression> left,
                                                          std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::not_equal_to, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> less(std::unique_ptr<expression> left,
                                                  std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::less, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> greater(std::unique_ptr<expression> left,
                                                     std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::greater, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> less_equal(std::unique_ptr<expression> left,
                                                        std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::less_equal, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> greater_equal(std::unique_ptr<expression> left,
                                                           std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::greater_equal, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> logical_and(std::unique_ptr<expression> left,
                                                         std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::logical_and, std::move(left), std::move(right));
}

inline std::unique_ptr<binary_operator_expr> logical_or(std::unique_ptr<expression> left,
                                                        std::unique_ptr<expression> right)
{
    return binary_operator(binary_op_tag::logical_or, std::move(left), std::move(right));
}
}
}

#endif