#include <qubus/qtl/lower_top_level_sums.hpp>

#include <qubus/IR/qir.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/qtl/pattern/all.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <mutex>
#include <string>
#include <vector>

namespace qubus
{
namespace qtl
{

namespace
{

std::unique_ptr<expression> remove_top_level_sums(const expression& expr,
                                                  std::vector<variable_declaration>& sum_indices)
{
    using qubus::pattern::variable;
    using pattern::sum;

    variable<const expression&> body;
    variable<variable_declaration> index;

    auto m = qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>()
                 .case_(sum(body, index),
                        [&] {
                            sum_indices.push_back(index.get());

                            return remove_top_level_sums(body.get(), sum_indices);
                        })
                 .case_(body, [&] { return clone(body.get()); });

    return qubus::pattern::match(expr, m);
}
}

std::unique_ptr<expression> lower_top_level_sums(const expression& expr)
{
    using qubus::pattern::variable;
    using pattern::sum;

    variable<const expression &> a, b, c, d;
    variable<const sum_expr&> s;
    variable<variable_declaration> idx;
    variable<std::vector<std::reference_wrapper<expression>>> sub_exprs;
    variable<binary_op_tag> tag;

    using qubus::pattern::_;

    auto m = qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>()
                 .case_(pattern::for_all(idx, b),
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

    return qubus::pattern::match(expr, m);
}

}
}