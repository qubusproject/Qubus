#include <qbb/qubus/kronecker_delta_folding_pass.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <boost/optional.hpp>

#include <vector>
#include <algorithm>

namespace qbb
{
namespace qubus
{

namespace
{

using bubble_t = std::tuple<variable_declaration, variable_declaration>;

bubble_t make_bubble(variable_declaration first, variable_declaration second)
{
    return std::less<variable_declaration>()(first, second)
               ? std::make_tuple(std::move(first), std::move(second))
               : std::make_tuple(std::move(second), std::move(first));
}

struct collect_bubbles_context
{
    std::vector<bubble_t> bubbles;
};

void collect_bubbles(const expression& expr, collect_bubbles_context& ctx)
{
    using pattern::value;
    using pattern::_;

    pattern::variable<expression> a, b;
    pattern::variable<variable_declaration> i, j;
    pattern::variable<std::vector<variable_declaration>> indices;

    auto m =
        pattern::make_matcher<expression, void>()
            .case_(sum_multi(a, indices),
                   [&]
                   {
                       collect_bubbles(a.get(), ctx);

                       for (const auto& index : indices.get())
                       {
                           auto last = std::remove_if(ctx.bubbles.begin(), ctx.bubbles.end(),
                                                      [&](const bubble_t& value)
                                                      {
                                                          return index == std::get<0>(value) ||
                                                                 index == std::get<1>(value);
                                                      });

                           ctx.bubbles.erase(last, ctx.bubbles.end());
                       }
                   })
            .case_(binary_operator(value(binary_op_tag::plus) || value(binary_op_tag::minus), a, b),
                   [&]
                   {
                       collect_bubbles_context ctx_left;
                       collect_bubbles_context ctx_right;

                       collect_bubbles(a.get(), ctx_left);
                       collect_bubbles(b.get(), ctx_right);

                       for (const auto& bubble : ctx_left.bubbles)
                       {
                           auto iter = std::find(ctx_right.bubbles.begin(), ctx_right.bubbles.end(),
                                                 bubble);

                           if (iter != ctx_right.bubbles.end())
                           {
                               ctx.bubbles.push_back(bubble);
                           }
                       }
                   })
            .case_(binary_operator(value(binary_op_tag::divides), a, _),
                   [&]
                   {
                       collect_bubbles(a.get(), ctx);
                   })
            .case_(binary_operator(value(binary_op_tag::multiplies), a, b),
                   [&]
                   {
                       collect_bubbles(a.get(), ctx);
                       collect_bubbles(b.get(), ctx);
                   })
            .case_(delta(index(i), index(j)), [&]
                   {
                       // Eliminating contractions is only valid if the indices differ. Therefore,
                       // we only produce a bubble in this case.
                       if (i.get() != j.get())
                       {
                           ctx.bubbles.push_back(make_bubble(i.get(), j.get()));
                       }
                   });

    pattern::try_match(expr, m);
}

boost::optional<variable_declaration>
contains_bubble_with_index(const variable_declaration& index, const std::vector<bubble_t>& bubbles)
{
    for (const auto& bubble : bubbles)
    {
        if (std::get<0>(bubble) == index)
        {
            return std::get<1>(bubble);
        }
        else if (std::get<1>(bubble) == index)
        {
            return std::get<0>(bubble);
        }
    }

    return boost::none;
}

expression eliminate_trivial_deltas(const expression& expr)
{
    pattern::variable<variable_declaration> i;

    auto m3 = pattern::make_matcher<expression, expression>().case_(
            delta(index(i), index(i)), [&]
    {
        return integer_literal_expr(1);
    });

    return pattern::substitute(expr, m3);
}

expression substitute_index(const expression& expr, const variable_declaration& idx, const variable_declaration& other_idx)
{
    using pattern::value;

    auto m2 = pattern::make_matcher<expression, expression>().case_(
            index(value(idx)), [&]
    {
        return variable_ref_expr(other_idx);
    });

    return pattern::substitute(expr, m2);
}

}

expression fold_kronecker_deltas(expression expr)
{
    pattern::variable<expression> body;
    pattern::variable<std::vector<variable_declaration>> indices;

    auto m = pattern::make_matcher<expression, expression>().case_(
        sum_multi(body, indices), [&]
        {
            expression new_body = body.get();

            collect_bubbles_context ctx;

            collect_bubbles(new_body, ctx);

            std::vector<variable_declaration> surviving_indices;

            for (const auto& index : indices.get())
            {
                if (auto other_index = contains_bubble_with_index(index, ctx.bubbles))
                {
                    new_body = substitute_index(new_body, index, *other_index);

                    new_body = eliminate_trivial_deltas(new_body);
                }
                else
                {
                    surviving_indices.push_back(index);
                }
            }

            if (surviving_indices.empty())
            {
                return new_body;
            }
            else
            {
                return expression(sum_expr(surviving_indices, new_body));
            }
        });

    return pattern::substitute(expr, m);
}

function_declaration fold_kronecker_deltas(function_declaration decl)
{
    decl.substitute_body(fold_kronecker_deltas(decl.body()));

    return decl;
}
}
}