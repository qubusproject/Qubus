#include <qbb/qubus/IR/constant_folding.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <qbb/qubus/IR/type_inference.hpp>

#include <qbb/qubus/IR/qir.hpp>

#include <stack>

namespace qbb
{
namespace qubus
{

namespace
{

std::unique_ptr<expression> fold_impl(const expression& expr)
{
    using pattern::_;

    // TODO: Generalize this to arbitrary integer types.
    if (typeof_(expr) != types::integer{})
        return clone(expr);

    pattern::variable<binary_op_tag> btag;
    pattern::variable<unary_op_tag> utag;
    pattern::variable<util::index_t> i1, i2;

    auto m = pattern::make_matcher<expression, std::unique_ptr<expression>>()
                 .case_(binary_operator(btag, integer_literal(i1), integer_literal(i2)),
                        [&](const expression& self) -> std::unique_ptr<expression> {
                            switch (btag.get())
                            {
                            case binary_op_tag::plus:
                                return integer_literal(i1.get() + i2.get());
                            case binary_op_tag::minus:
                                return integer_literal(i1.get() - i2.get());
                            case binary_op_tag::multiplies:
                                return integer_literal(i1.get() * i2.get());
                            case binary_op_tag::divides:
                                return integer_literal(i1.get() / i2.get());
                            case binary_op_tag::modulus:
                                return integer_literal(i1.get() % i2.get());
                            case binary_op_tag::div_floor:
                            {
                                auto quotient = i1.get() / i2.get();
                                auto remainder = i1.get() % i2.get();

                                auto is_exact = remainder == 0;
                                auto is_positive = remainder < 0 == quotient < 0;
                                auto is_exact_or_positive = is_exact || is_positive;

                                auto reduced_quotient = quotient - 1;

                                return integer_literal(is_exact_or_positive ? quotient
                                                                            : reduced_quotient);
                            }
                            default:
                                return clone(self);
                            }
                        })
                 .case_(unary_operator(utag, integer_literal(i1)),
                        [&](const expression& self) -> std::unique_ptr<expression> {
                            switch (utag.get())
                            {
                            case unary_op_tag::plus:
                                return integer_literal(i1.get());
                            case unary_op_tag::negate:
                                return integer_literal(-i1.get());
                            default:
                                return clone(self);
                            }
                        })
                 .case_(_, [](const expression& self) { return clone(self); });

    return pattern::match(expr, m);
}
}

std::unique_ptr<expression> fold_constant_expressions(const expression& expr)
{
    std::vector<std::unique_ptr<expression>> simplified_children;
    simplified_children.reserve(expr.arity());

    for (const auto& child : expr.sub_expressions())
    {
        simplified_children.push_back(fold_constant_expressions(child));
    }

    auto new_expr = expr.substitute_subexpressions(std::move(simplified_children));

    return fold_impl(*new_expr);
}
}
}
