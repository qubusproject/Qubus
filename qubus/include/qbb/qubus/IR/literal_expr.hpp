#ifndef QBB_QUBUS_LITERAL_EXPR_HPP
#define QBB_QUBUS_LITERAL_EXPR_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/annotations.hpp>
#include <qbb/qubus/IR/expression.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/unused.hpp>

#include <type_traits>
#include <vector>

inline namespace qbb
{
namespace qubus
{

class double_literal_expr : public expression_base<double_literal_expr>
{

public:
    double_literal_expr() = default;
    explicit double_literal_expr(double value_);

    virtual ~double_literal_expr() = default;

    double value() const;

    double_literal_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& value_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(double_literal_expr);

private:
    double value_;

    mutable annotation_map annotations_;
};

bool operator==(const double_literal_expr& lhs, const double_literal_expr& rhs);
bool operator!=(const double_literal_expr& lhs, const double_literal_expr& rhs);

inline std::unique_ptr<double_literal_expr> double_literal(double value)
{
    return std::make_unique<double_literal_expr>(value);
}

inline std::unique_ptr<double_literal_expr> lit(double value)
{
    return double_literal(value);
}

class float_literal_expr : public expression_base<float_literal_expr>
{

public:
    float_literal_expr() = default;
    explicit float_literal_expr(float value_);

    virtual ~float_literal_expr() = default;

    float value() const;

    float_literal_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& value_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(float_literal_expr);

private:
    float value_;

    mutable annotation_map annotations_;
};

bool operator==(const float_literal_expr& lhs, const float_literal_expr& rhs);
bool operator!=(const float_literal_expr& lhs, const float_literal_expr& rhs);

inline std::unique_ptr<float_literal_expr> float_literal(float value)
{
    return std::make_unique<float_literal_expr>(value);
}

inline std::unique_ptr<float_literal_expr> lit(float value)
{
    return float_literal(value);
}

class integer_literal_expr : public expression_base<integer_literal_expr>
{

public:
    integer_literal_expr() = default;
    explicit integer_literal_expr(qbb::util::index_t value_);

    virtual ~integer_literal_expr() = default;

    qbb::util::index_t value() const;

    integer_literal_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& value_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(integer_literal_expr);

private:
    qbb::util::index_t value_;
};

bool operator==(const integer_literal_expr& lhs, const integer_literal_expr& rhs);
bool operator!=(const integer_literal_expr& lhs, const integer_literal_expr& rhs);

inline std::unique_ptr<integer_literal_expr> integer_literal(qbb::util::index_t value)
{
    return std::make_unique<integer_literal_expr>(value);
}

template <typename Integral>
typename std::enable_if<std::is_integral<Integral>::value,
                        std::unique_ptr<integer_literal_expr>>::type
lit(Integral value)
{
    return integer_literal(value);
}
}
}

#endif