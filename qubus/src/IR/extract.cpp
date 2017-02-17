#include <qubus/IR/extract.hpp>

#include <qubus/IR/deduce_expression_environment.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/IR/qir.hpp>

#include <boost/optional.hpp>

#include <qubus/util/assert.hpp>

#include <qubus/IR/pretty_printer.hpp>

#include <map>
#include <vector>

namespace qubus
{

namespace
{
std::unique_ptr<expression> substitite_env_vars(const expression& expr,
                                                const std::vector<parameter>& env_vars,
                                                std::vector<parameter>& substituion_vars)
{
    std::unique_ptr<expression> new_expr = clone(expr);

    std::map<util::handle, variable_declaration> substitution_table;

    for (const auto& var : env_vars)
    {
        variable_declaration substitute_var(var.declaration().var_type());

        substitution_table.emplace(var.declaration().id(), substitute_var);

        substituion_vars.emplace_back(var.intent(), substitute_var);
    }

    for (const auto& var : env_vars)
    {
        auto m = pattern::make_matcher<expression, std::unique_ptr<expression>>().case_(
            variable_ref(pattern::value(var.declaration())), [&] {
                const auto& substitute_var = substitution_table.at(var.declaration().id());

                return std::make_unique<variable_ref_expr>(substitute_var);
            });

        new_expr = pattern::substitute(expr, m);
    }

    return new_expr;
}
}

std::unique_ptr<expression> extract_expr_as_function(const expression& expr,
                                                     const std::string& extracted_func_name)
{
    auto expr_env = deduce_expression_environment(expr);

    std::vector<parameter> params;

    auto body = substitite_env_vars(expr, expr_env.parameters(), params);

    QUBUS_ASSERT(params.size() == expr_env.parameters().size(), "");

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
        pretty_print(*body);
        throw 0; // Invalid number of out parameters.
    }

    function_declaration extracted_function(extracted_func_name, in_params, out_params[0], std::move(body));

    std::vector<std::unique_ptr<expression>> args;

    std::unique_ptr<expression> result;

    for (const auto& param : expr_env.parameters())
    {
        if (param.intent() == variable_intent::in)
        {
            args.push_back(var(param.declaration()));
        }
        else if (param.intent() == variable_intent::out)
        {
            result = var(param.declaration());
        }
    }

    args.push_back(std::move(result));

    return spawn(std::move(extracted_function), std::move(args));
}
}