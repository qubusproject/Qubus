#include <qbb/qubus/make_implicit_conversions_explicit.hpp>

#include <qbb/qubus/IR/type_inference.hpp>

#include <qbb/qubus/IR/qir.hpp>

#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

inline namespace qbb
{
namespace qubus
{

namespace
{
std::unique_ptr<expression> make_implicit_conversions_explicit(const expression& expr)
{
    // TODO: add implicit conversions for intrinsic functions

    using pattern::_;

    pattern::variable<binary_op_tag> btag;
    pattern::variable<unary_op_tag> utag;
    pattern::variable<const expression&> a, b;
    pattern::variable<type> t;

    auto m = pattern::make_matcher<expression, std::unique_ptr<expression>>()
                 .case_(binary_operator(btag, a, b),
                        [&] (const expression& self) -> std::unique_ptr<expression>
                        {
                            switch (btag.get())
                            {
                                case binary_op_tag::equal_to:
                                case binary_op_tag::less_equal:
                                case binary_op_tag::less:
                                case binary_op_tag::greater_equal:
                                case binary_op_tag::greater:
                                    return clone(self);
                                default:
                                {
                                    auto result_type = typeof_(self);
                                    auto lhs_type = typeof_(a.get());
                                    auto rhs_type = typeof_(b.get());

                                    auto new_lhs = [&] () -> std::unique_ptr<expression>
                                    {
                                        if (lhs_type != result_type)
                                        {
                                            return type_conversion(result_type, clone(a.get()));
                                        }
                                        else
                                        {
                                            return clone(a.get());
                                        }
                                    }();

                                    auto new_rhs = [&] () -> std::unique_ptr<expression>
                                    {
                                        if (rhs_type != result_type)
                                        {
                                            return type_conversion(result_type, clone(b.get()));
                                        }
                                        else
                                        {
                                            return clone(b.get());
                                        }
                                    }();

                                    return binary_operator(btag.get(), std::move(new_lhs), std::move(new_rhs));
                                }
                            }
                        })
                 .case_(unary_operator(utag, a), [&] (const expression& self) -> std::unique_ptr<expression>
                        {
                            auto result_type = typeof_(self);
                            auto arg_type = typeof_(a.get());

                            auto new_arg = [&] () -> std::unique_ptr<expression>
                            {
                                if (result_type != arg_type)
                                {
                                    return type_conversion(result_type, clone(a.get()));
                                }
                                else
                                {
                                    return clone(a.get());
                                }
                            }();

                            return unary_operator(utag.get(), std::move(new_arg));
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