#include <qbb/qubus/make_implicit_conversions_explicit.hpp>

#include <qbb/qubus/IR/type_inference.hpp>

#include <qbb/qubus/IR/kir.hpp>

#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

namespace qbb
{
namespace qubus
{

namespace
{
expression make_implicit_conversions_explicit(expression expr)
{
    // TODO: add implicit conversions for intrinsic functions

    using pattern::_;

    pattern::variable<binary_op_tag> btag;
    pattern::variable<unary_op_tag> utag;
    pattern::variable<expression> current_expr, a, b;
    pattern::variable<type> t;

    auto m = pattern::make_matcher<expression, expression>()
                 .case_(bind_to(binary_operator(btag, a, b), current_expr),
                        [&]
                        {
                            switch (btag.get())
                            {
                                case binary_op_tag::equal_to:
                                case binary_op_tag::less_equal:
                                case binary_op_tag::less:
                                case binary_op_tag::greater_equal:
                                case binary_op_tag::greater:
                                    return current_expr.get();
                                default:
                                {
                                    auto result_type = typeof_(current_expr.get());
                                    auto lhs_type = typeof_(a.get());
                                    auto rhs_type = typeof_(b.get());

                                    auto new_lhs = [&]
                                    {
                                        if (lhs_type != result_type)
                                        {
                                            return expression(type_conversion_expr(result_type, a.get()));
                                        }
                                        else
                                        {
                                            return a.get();
                                        }
                                    }();

                                    auto new_rhs = [&]
                                    {
                                        if (rhs_type != result_type)
                                        {
                                            return expression(type_conversion_expr(result_type, b.get()));
                                        }
                                        else
                                        {
                                            return b.get();
                                        }
                                    }();

                                    return expression(binary_operator_expr(btag.get(), new_lhs, new_rhs));
                                }
                            }
                        })
                 .case_(bind_to(unary_operator(utag, a), current_expr), [&]
                        {
                            auto result_type = typeof_(current_expr.get());
                            auto arg_type = typeof_(a.get());

                            auto new_arg = [&]
                            {
                                if (result_type != arg_type)
                                {
                                    return expression(type_conversion_expr(result_type, a.get()));
                                }
                                else
                                {
                                    return a.get();
                                }
                            }();

                            return unary_operator_expr(utag.get(), new_arg);
                        });

    return pattern::substitute(expr, m);
}
}

function_declaration make_implicit_conversions_explicit(function_declaration decl)
{
    decl.substitute_body(make_implicit_conversions_explicit(decl.body()));
    
    return decl;
}
}
}