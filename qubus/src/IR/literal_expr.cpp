#include <qubus/IR/literal_expr.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/unused.hpp>

namespace qubus
{

double_literal_expr::double_literal_expr(double value_) : value_{value_}
{
}

double double_literal_expr::value() const
{
    return value_;
}

double_literal_expr* double_literal_expr::clone() const
{
    return new double_literal_expr(value_);
}

const expression& double_literal_expr::child(std::size_t QBB_UNUSED(index)) const
{
    throw 0;
}

std::size_t double_literal_expr::arity() const
{
    return 0;
}

std::unique_ptr<expression> double_literal_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 0)
        throw 0;

    return std::make_unique<double_literal_expr>(value_);
}

bool operator==(const double_literal_expr& lhs, const double_literal_expr& rhs)
{
    return lhs.value() == rhs.value();
}

bool operator!=(const double_literal_expr& lhs, const double_literal_expr& rhs)
{
    return !(lhs == rhs);
}

float_literal_expr::float_literal_expr(float value_) : value_{value_}
{
}

float float_literal_expr::value() const
{
    return value_;
}

float_literal_expr* float_literal_expr::clone() const
{
    return new float_literal_expr(value_);
}

const expression& float_literal_expr::child(std::size_t QBB_UNUSED(index)) const
{
    throw 0;
}

std::size_t float_literal_expr::arity() const
{
    return 0;
}

std::unique_ptr<expression> float_literal_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 0)
        throw 0;

    return std::make_unique<float_literal_expr>(value_);
}

bool operator==(const float_literal_expr& lhs, const float_literal_expr& rhs)
{
    return lhs.value() == rhs.value();
}

bool operator!=(const float_literal_expr& lhs, const float_literal_expr& rhs)
{
    return !(lhs == rhs);
}

integer_literal_expr::integer_literal_expr(util::index_t value_) : value_{value_}
{
}

util::index_t integer_literal_expr::value() const
{
    return value_;
}

integer_literal_expr* integer_literal_expr::clone() const
{
    return new integer_literal_expr(value_);
}

const expression& integer_literal_expr::child(std::size_t QBB_UNUSED(index)) const
{
    throw 0;
}

std::size_t integer_literal_expr::arity() const
{
    return 0;
}

std::unique_ptr<expression> integer_literal_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 0)
        throw 0;

    return std::make_unique<integer_literal_expr>(value_);
}

bool operator==(const integer_literal_expr& lhs, const integer_literal_expr& rhs)
{
    return lhs.value() == rhs.value();
}

bool operator!=(const integer_literal_expr& lhs, const integer_literal_expr& rhs)
{
    return !(lhs == rhs);
}
}