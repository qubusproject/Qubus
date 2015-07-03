#include <qbb/qubus/IR/extract.hpp>

#include <qbb/qubus/IR/deduce_expression_environment.hpp>

#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <qbb/qubus/IR/qir.hpp>

#include <boost/optional.hpp>

#include <qbb/util/assert.hpp>

#include <qbb/qubus/IR/pretty_printer.hpp>

#include <map>
#include <vector>

namespace qbb
{
namespace qubus
{

namespace
{
expression substitite_env_vars(expression expr, const std::vector<parameter>& env_vars,
                               std::vector<parameter>& substituion_vars)
{
    std::map<util::handle, variable_declaration> substitution_table;

    for (const auto& var : env_vars)
    {
        variable_declaration substitute_var(var.declaration().var_type());

        substitution_table.emplace(var.declaration().id(), substitute_var);

        substituion_vars.emplace_back(var.intent(), substitute_var);
    }

    for (const auto& var : env_vars)
    {
        auto m = pattern::make_matcher<expression, expression>().case_(
            variable_ref(pattern::value(var.declaration())), [&]
            {
                const auto& substitute_var = substitution_table.at(var.declaration().id());

                return variable_ref_expr(substitute_var);
            });

        expr = pattern::substitute(expr, m);
    }

    return expr;
}
}

expression extract_expr_as_function(expression expr, const std::string& extracted_func_name)
{
    auto expr_env = deduce_expression_environment(expr);

    std::vector<parameter> params;

    expr = substitite_env_vars(expr, expr_env.parameters(), params);

    QBB_ASSERT(params.size() == expr_env.parameters().size(), "");

    std::vector<variable_declaration> in_params;
    std::vector<variable_declaration> out_params;

    for (const auto& param : params)
    {
        if (param.intent() == variable_intent::in)
        {
            in_params.push_back(param.declaration());
        }
        else if (param.intent() == variable_intent::out)
        {
            out_params.push_back(param.declaration());
        }
    }

    if (out_params.size() != 1)
    {
        pretty_print(expr);
        throw 0; // Invalid number of out parameters.
    }

    function_declaration extracted_function(extracted_func_name, in_params, out_params[0], expr);

    std::vector<expression> args;

    boost::optional<expression> result;

    for (const auto& param : expr_env.parameters())
    {
        if (param.intent() == variable_intent::in)
        {
            args.push_back(variable_ref_expr(param.declaration()));
        }
        else if (param.intent() == variable_intent::out)
        {
            result = variable_ref_expr(param.declaration());
        }
    }

    args.push_back(*result);

    return spawn_expr(extracted_function, args);
}
}
}