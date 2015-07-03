#include <qbb/kubus/IR/literal_expr.hpp>

#include <qbb/util/assert.hpp>
#include <qbb/util/unused.hpp>

namespace qbb
{
namespace qubus
{

double_literal_expr::double_literal_expr(double value_) : value_{value_}
{
}

double double_literal_expr::value() const
{
    return value_;
}

std::vector<expression> double_literal_expr::sub_expressions() const
{
    return {};
}

expression double_literal_expr::substitute_subexpressions(
    const std::vector<expression>& QBB_UNUSED_RELEASE(subexprs)) const
{
    QBB_ASSERT(subexprs.size() == 0, "invalid number of subexpressions");

    return double_literal_expr(value_);
}

annotation_map& double_literal_expr::annotations() const
{
    return annotations_;
}

annotation_map& double_literal_expr::annotations()
{
    return annotations_;
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

std::vector<expression> float_literal_expr::sub_expressions() const
{
    return {};
}

expression float_literal_expr::substitute_subexpressions(
    const std::vector<expression>& QBB_UNUSED_RELEASE(subexprs)) const
{
    QBB_ASSERT(subexprs.size() == 0, "invalid number of subexpressions");

    return float_literal_expr(value_);
}

annotation_map& float_literal_expr::annotations() const
{
    return annotations_;
}

annotation_map& float_literal_expr::annotations()
{
    return annotations_;
}

bool operator==(const float_literal_expr& lhs, const float_literal_expr& rhs)
{
    return lhs.value() == rhs.value();
}

bool operator!=(const float_literal_expr& lhs, const float_literal_expr& rhs)
{
    return !(lhs == rhs);
}

integer_literal_expr::integer_literal_expr(qbb::util::index_t value_) : value_{value_}
{
}

qbb::util::index_t integer_literal_expr::value() const
{
    return value_;
}

std::vector<expression> integer_literal_expr::sub_expressions() const
{
    return {};
}

expression integer_literal_expr::substitute_subexpressions(
    const std::vector<expression>& QBB_UNUSED_RELEASE(subexprs)) const
{
    QBB_ASSERT(subexprs.size() == 0, "invalid number of subexpressions");

    return integer_literal_expr(value_);
}

annotation_map& integer_literal_expr::annotations() const
{
    return annotations_;
}

annotation_map& integer_literal_expr::annotations()
{
    return annotations_;
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
}