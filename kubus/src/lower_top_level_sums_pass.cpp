#include <qbb/kubus/lower_top_level_sums_pass.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/IR.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <vector>
#include <string>
#include <mutex>

namespace qbb
{
namespace kubus
{

namespace
{

expression remove_top_level_sums(const expression& expr, std::vector<std::string>& sum_indices)
{
    pattern::variable<expression> body;
    pattern::variable<std::vector<expression>> indices;

    auto m = pattern::make_matcher<expression, expression>()
                 .case_(sum(body, indices), [&]
                        {
                     for (const auto& index : indices.get())
                     {
                         sum_indices.push_back(index.as<index_expr>().id());
                     }

                     return remove_top_level_sums(body.get(), sum_indices);
                 })
                 .case_(body, [&]
                        {
                     return body.get();
                 });

    return pattern::match(expr, m);
}
}

expression lower_top_level_sums(const expression& expr)
{
    pattern::variable<expression> a, b, c, d;
    pattern::variable<std::vector<expression>> sub_exprs;
    pattern::variable<binary_op_tag> tag;

    auto m = pattern::make_matcher<expression, expression>()
                 .case_(for_all(a, b), [&]
                        {
                     return for_all_expr(a.get(), lower_top_level_sums(b.get()));
                 })
                 .case_(for_(a, b, c, d), [&]
                        {
                     return for_expr(a.get(), b.get(), c.get(), lower_top_level_sums(d.get()));
                 })
                 .case_(compound(sub_exprs), [&]
                        {
                     std::vector<expression> new_sub_expressions;

                     for (const auto& sub_expr : sub_exprs.get())
                     {
                         new_sub_expressions.push_back(lower_top_level_sums(sub_expr));
                     }

                     return compound_expr(new_sub_expressions);
                 })
                 .case_(binary_operator(tag, a, b), [&]
                        {
                     if (tag.get() == binary_op_tag::assign)
                     {
                         std::vector<std::string> sum_index_ids;

                         auto new_rhs = remove_top_level_sums(b.get(), sum_index_ids);

                         auto initializer = binary_operator_expr(binary_op_tag::assign, a.get(),
                                                                 integer_literal_expr(0));

                         expression new_expr =
                             binary_operator_expr(binary_op_tag::plus_assign, a.get(), new_rhs);

                         for (const auto& sum_index_id : sum_index_ids | boost::adaptors::reversed)
                         {
                             new_expr = for_all_expr(index_expr(sum_index_id), new_expr);
                         }

                         return expression(compound_expr({std::move(initializer), std::move(new_expr)}));
                     }
                     else
                     {
                         return expr;
                     }
                 });

    return pattern::match(expr, m);
}

}
}