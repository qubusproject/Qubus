#include <qbb/kubus/lower_top_level_sums.hpp>

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

expression remove_top_level_sums(const expression& expr, std::vector<variable_declaration>& sum_indices)
{
    pattern::variable<expression> body;
    pattern::variable<variable_declaration> index;

    auto m = pattern::make_matcher<expression, expression>()
                 .case_(sum(body, index), [&]
                        {
                     sum_indices.push_back(index.get());

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
    pattern::variable<sum_expr> s;
    pattern::variable<variable_declaration> idx;
    pattern::variable<std::vector<expression>> sub_exprs;
    pattern::variable<binary_op_tag> tag;

    using pattern::_;
    
    auto m = pattern::make_matcher<expression, expression>()
                 .case_(for_all(idx, b), [&]
                        {
                     return for_all_expr(idx.get(), lower_top_level_sums(b.get()));
                 })
                 .case_(for_(idx, b, c, d), [&]
                        {
                     return for_expr(idx.get(), b.get(), c.get(), lower_top_level_sums(d.get()));
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
                 .case_(binary_operator(pattern::value(binary_op_tag::assign), a, bind_to(sum(_, _), s)), [&]
                        {
                         std::vector<variable_declaration> sum_indices;

                         auto new_rhs = remove_top_level_sums(s.get(), sum_indices);

                         auto initializer = binary_operator_expr(binary_op_tag::assign, a.get(),
                                                                 double_literal_expr(0));

                         expression new_expr =
                             binary_operator_expr(binary_op_tag::plus_assign, a.get(), new_rhs);

                         for (const auto& sum_index : sum_indices | boost::adaptors::reversed)
                         {
                             new_expr = for_all_expr(sum_index, new_expr);
                         }

                         return expression(compound_expr({std::move(initializer), std::move(new_expr)}));
                 })
                 .case_(a, [&]
                 {
                     return a.get();
                 });

    return pattern::match(expr, m);
}

function_declaration lower_top_level_sums(const function_declaration& decl)
{
    auto new_body = lower_top_level_sums(decl.body());
    
    return function_declaration(decl.params(), decl.result(), new_body);
}

}
}