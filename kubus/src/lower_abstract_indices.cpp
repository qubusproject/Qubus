#include <qbb/kubus/lower_abstract_indices.hpp>

#include <qbb/kubus/IR/kir.hpp>
#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/IR.hpp>
#include <qbb/kubus/pattern/substitute.hpp>

#include <qbb/kubus/deduce_iteration_space.hpp>

#include <qbb/util/handle.hpp>

#include <boost/optional.hpp>

#include <array>
#include <utility>

namespace qbb
{
namespace kubus
{

namespace
{

expression lower_abstract_indices_impl(const expression& expr)
{
    using pattern::_;

    pattern::variable<variable_declaration> decl;
    pattern::variable<expression> body;

    auto m = pattern::make_matcher<expression, expression>().case_(for_all(decl, body),
                                                                   [&]() -> expression
                                                                   {

        using pattern::value;
                                                                       
        auto bounds = deduce_iteration_space(decl.get(), body.get());

        expression lower_bound = bounds[0];
        expression upper_bound = bounds[1];

        variable_declaration loop_index{types::integer()};
        loop_index.annotations().add("kubus.debug.name", decl.get().annotations().lookup("kubus.debug.name"));
 
        auto m2 = pattern::make_matcher<expression, expression>().case_(index(protect(decl)), [&]
                                                                        {
            return variable_ref_expr(loop_index);
        });

        auto new_body = pattern::substitute(body.get(), m2);

        return for_expr(loop_index, lower_bound, upper_bound, new_body);
    });

    return pattern::substitute(expr, m);
}
}

expression lower_abstract_indices(const expression& expr)
{
    return lower_abstract_indices_impl(expr);
}

function_declaration lower_abstract_indices(const function_declaration& decl)
{
    auto new_body = lower_abstract_indices(decl.body());

    return function_declaration(decl.params(), decl.result(), new_body);
}
}
}