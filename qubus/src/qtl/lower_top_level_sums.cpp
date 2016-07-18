#include <qbb/qubus/qtl/lower_top_level_sums.hpp>

#include <qbb/qubus/IR/qir.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <mutex>
#include <string>
#include <vector>

namespace qbb
{
namespace qubus
{
namespace qtl
{

namespace
{

std::unique_ptr<expression> remove_top_level_sums(const expression& expr,
                                                  std::vector<variable_declaration>& sum_indices)
{
    pattern::variable<const expression&> body;
    pattern::variable<variable_declaration> index;

    auto m = pattern::make_matcher<expression, std::unique_ptr<expression>>()
                 .case_(sum(body, index),
                        [&] {
                            sum_indices.push_back(index.get());

                            return remove_top_level_sums(body.get(), sum_indices);
                        })
                 .case_(body, [&] { return clone(body.get()); });

    return pattern::match(expr, m);
}
}

std::unique_ptr<expression> lower_top_level_sums(const expression& expr)
{
    pattern::variable<const expression &> a, b, c, d;
    pattern::variable<const sum_expr&> s;
    pattern::variable<variable_declaration> idx;
    pattern::variable<std::vector<std::reference_wrapper<expression>>> sub_exprs;
    pattern::variable<binary_op_tag> tag;

    using pattern::_;

    auto m = pattern::make_matcher<expression, std::unique_ptr<expression>>()
                 .case_(for_all(idx, b),
                        [&] { return for_all(idx.get(), lower_top_level_sums(b.get())); })
                 .case_(for_(idx, b, c, d),
                        [&] {
                            return for_(idx.get(), clone(b.get()), clone(c.get()),
                                        lower_top_level_sums(d.get()));
                        })
                 .case_(compound(sub_exprs),
                        [&] {
                            std::vector<std::unique_ptr<expression>> new_sub_expressions;

                            for (const auto& sub_expr : sub_exprs.get())
                            {
                                new_sub_expressions.push_back(lower_top_level_sums(sub_expr));
                            }

                            return sequenced_tasks(std::move(new_sub_expressions));
                        })
                 .case_(assign(a, bind_to(sum(_, _), s)),
                        [&] {
                            std::vector<variable_declaration> sum_indices;

                            auto new_rhs = remove_top_level_sums(s.get(), sum_indices);

                            auto initializer = assign(clone(a.get()), double_literal(0));

                            std::unique_ptr<expression> new_expr =
                                plus_assign(clone(a.get()), std::move(new_rhs));

                            for (const auto& sum_index : sum_indices | boost::adaptors::reversed)
                            {
                                new_expr = for_all(sum_index, std::move(new_expr));
                            }

                            return sequenced_tasks(std::move(initializer), std::move(new_expr));
                        })
                 .case_(a, [&] { return clone(a.get()); });

    return pattern::match(expr, m);
}

function_declaration lower_top_level_sums(function_declaration decl)
{
    auto new_body = lower_top_level_sums(decl.body());

    decl.substitute_body(std::move(new_body));

    return decl;
}
}
}
}