#include <qbb/qubus/lower_abstract_indices.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/substitute.hpp>

#include <qbb/qubus/deduce_iteration_space.hpp>

#include <qbb/util/handle.hpp>

#include <boost/optional.hpp>

#include <array>
#include <utility>

namespace qbb
{
namespace qubus
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
        loop_index.annotations().add("qubus.debug.name", decl.get().annotations().lookup("qubus.debug.name"));
 
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

function_declaration lower_abstract_indices(function_declaration decl)
{
    auto new_body = lower_abstract_indices(decl.body());

    decl.substitute_body(std::move(new_body));
    
    return decl;
}
}
}