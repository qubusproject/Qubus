#include <qbb/qubus/deduce_iteration_space.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <cstddef>

namespace qbb
{
namespace qubus
{

std::array<expression, 2> deduce_iteration_space(const variable_declaration& idx,
                                                 const expression& expr)
{
    using pattern::value;
    using pattern::_;

    pattern::variable<variable_declaration> decl;
    pattern::variable<std::size_t> index_pos;

    pattern::variable<util::index_t> extent;

    auto m = pattern::make_matcher<expression, std::array<expression, 2>>()
                 .case_(subscription(tensor(decl), bind_to(any_of(index(value(idx))), index_pos)),
                        [&]
                        {
                            expression lower_bound = integer_literal_expr(0);

                            expression tensor = variable_ref_expr(decl.get());
                            expression index_position = integer_literal_expr(index_pos.get());
                            expression upper_bound =
                                intrinsic_function_expr("extent", {tensor, index_position});

                            return std::array<expression, 2>{{lower_bound, upper_bound}};
                        })
                 .case_(delta(extent, index(value(idx)), _) || delta(extent, _, index(value(idx))),
                        [&]
                        {
                            expression lower_bound = integer_literal_expr(0);
                            expression upper_bound = integer_literal_expr(extent.get());

                            return std::array<expression, 2>{{lower_bound, upper_bound}};
                        });

    auto result = pattern::search(expr, m);

    if (result)
    {
        return *result;
    }
    else
    {
        throw 0;
    }
}
}
}