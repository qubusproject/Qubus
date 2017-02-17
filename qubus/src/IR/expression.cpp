#include <qubus/IR/expression.hpp>

#include <qubus/IR/qir.hpp>

#include <qubus/pattern/core.hpp>
#include <qubus/pattern/IR.hpp>

#include <mutex>
#include <functional>

namespace qubus
{

expression_cursor::expression_cursor(const expression& expr_) : expr_(&expr_)
{
}

expression_cursor& expression_cursor::move_up()
{
    expr_ = expr_->parent();

    return *this;
}

expression_cursor& expression_cursor::move_down(std::size_t index)
{
    if (index < expr_->arity())
    {
        expr_ = &expr_->child(index);
    }
    else
    {
        expr_ = nullptr;
    }

    return *this;
}

std::size_t expression_cursor::arity() const
{
    return expr_->arity();
}

expression_cursor::operator bool() const
{
    return expr_;
}

const expression& expression_cursor::operator*() const
{
    return *expr_;
}

const expression* expression_cursor::operator->() const
{
    return expr_;
}

bool operator==(const expression_cursor& lhs, const expression_cursor& rhs)
{
    return &*lhs == &*rhs;
}

bool operator!=(const expression_cursor& lhs, const expression_cursor& rhs)
{
    return !(lhs == rhs);
}

util::implementation_table expression::implementation_table_ = {};

util::multi_method<bool(const util::virtual_<expression>&,
                             const util::virtual_<expression>&)> equal = {};

namespace
{

void init_equal()
{
    equal.add_specialization(std::equal_to<binary_operator_expr>());
    equal.add_specialization(std::equal_to<unary_operator_expr>());
    equal.add_specialization(std::equal_to<float_literal_expr>());
    equal.add_specialization(std::equal_to<double_literal_expr>());
    equal.add_specialization(std::equal_to<integer_literal_expr>());
    equal.add_specialization(std::equal_to<compound_expr>());
    equal.add_specialization(std::equal_to<for_expr>());
    equal.add_specialization(std::equal_to<intrinsic_function_expr>());
    equal.add_specialization(std::equal_to<subscription_expr>());
    equal.add_specialization(std::equal_to<type_conversion_expr>());
    equal.add_specialization(std::equal_to<variable_ref_expr>());

    equal.set_fallback([](const expression&, const expression&)
                       {
                           return false;
                       });
}

std::once_flag equal_init_flag = {};
}

bool operator==(const expression& lhs, const expression& rhs)
{
    std::call_once(equal_init_flag, init_equal);

    return equal(lhs, rhs);
}

bool operator!=(const expression& lhs, const expression& rhs)
{
    return !(lhs == rhs);
}

std::unique_ptr<expression> clone(const expression& expr)
{
    return std::unique_ptr<expression>(expr.clone());
}

std::vector<std::unique_ptr<expression>> clone(const std::vector<std::unique_ptr<expression>>& expressions)
{
    std::vector<std::unique_ptr<expression>> result;
    result.reserve(expressions.size());

    for (const auto& expression : expressions)
    {
        result.push_back(clone(*expression));
    }

    return result;
}

std::vector<std::unique_ptr<expression>> clone(const std::vector<std::reference_wrapper<expression>>& expressions)
{
    std::vector<std::unique_ptr<expression>> result;
    result.reserve(expressions.size());

    for (const auto& expression : expressions)
    {
        result.push_back(clone(expression));
    }

    return result;
}

}