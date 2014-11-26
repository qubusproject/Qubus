#include <qbb/kubus/deduce_iteration_space.hpp>

#include <qbb/kubus/pattern/IR.hpp>
#include <qbb/kubus/pattern/core.hpp>

#include <cstddef>

namespace qbb
{
namespace kubus
{

std::array<expression, 2> deduce_iteration_space(const variable_declaration& idx,
                                                 const expression& expr)
{
    using pattern::value;
    
    pattern::variable<variable_declaration> decl;
    pattern::variable<std::size_t> index_pos;

    auto m = pattern::make_matcher<expression, std::array<expression, 2>>().case_(
        subscription(tensor(decl), bind_to(any_of(index(value(idx))), index_pos)), [&]
        {
            expression lower_bound = integer_literal_expr(0);

            expression tensor = variable_ref_expr(decl.get());
            expression index_position = integer_literal_expr(index_pos.get());
            expression upper_bound = intrinsic_function_expr("extent", {tensor, index_position});

            return std::array<expression, 2>{{lower_bound, upper_bound}};
        });
    
    auto result = pattern::search(expr, m);
    
    if(result)
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