#include <qbb/kubus/IR/deduce_expression_environment.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/IR.hpp>

#include <boost/range/adaptor/map.hpp>

#include <qbb/util/box.hpp>

#include <utility>
#include <map>
#include <vector>

namespace qbb
{
namespace kubus
{

namespace
{

void extract_expression_parameter(const expression& expr,
                                  std::map<util::handle, util::box<parameter>>& parameter_table, bool is_modifing)
{
    using pattern::_;
    using pattern::value;

    pattern::variable<variable_declaration> var;
    pattern::variable<expression> scope;

    pattern::variable<expression> a, b;
    pattern::variable<std::vector<expression>> exprs;

    auto m =
        pattern::make_matcher<expression, void>()
            .case_(variable_scope(var, scope),
                   [&](const expression& self)
                   {
                       for (const auto& expr : self.sub_expressions())
                       {
                           extract_expression_parameter(expr, parameter_table, false);
                       }

                       parameter_table.erase(var.get().id());
                   })
            .case_(
                 binary_operator(value(binary_op_tag::assign) || value(binary_op_tag::plus_assign),
                                 a, b),
                 [&]
                 {
                     extract_expression_parameter(a.get(), parameter_table, true);
                     extract_expression_parameter(b.get(), parameter_table, false);
                 })
            .case_(subscription(a, exprs),
                   [&]
                   {
                       extract_expression_parameter(a.get(), parameter_table, is_modifing);

                       for (const auto& expr : exprs.get())
                       {
                           extract_expression_parameter(expr, parameter_table, false);
                       }
                   })
            .case_(variable_ref(var),
                   [&]
                   {
                       auto& param = parameter_table[var.get().id()];

                       if (param)
                       {
                           if (is_modifing && param->intent() == variable_intent::in)
                           {
                               param.replace(parameter(variable_intent::out, param->declaration()));
                           }
                       }
                       else
                       {
                           if (is_modifing)
                           {
                               param.fill(parameter(variable_intent::out, var.get()));
                           }
                           else
                           {
                               param.fill(parameter(variable_intent::in, var.get()));
                           }
                       }
                   })
            .case_(_, [&](const expression& self)
                   {
                       for (const expression& expr : self.sub_expressions())
                       {
                           extract_expression_parameter(expr, parameter_table, is_modifing);
                       }
                   });

    pattern::match(expr, m);
}
}

void expression_environment::add_parameter(parameter p)
{
    expression_parameters_.push_back(std::move(p));
}

const std::vector<parameter>& expression_environment::parameters() const
{
    return expression_parameters_;
}

expression_environment deduce_expression_environment(const expression& expr)
{
    expression_environment env;

    std::map<util::handle, util::box<parameter>> parameter_table;

    extract_expression_parameter(expr, parameter_table, false);

    for (const auto& param : parameter_table | boost::adaptors::map_values)
    {
        env.add_parameter(*param);
    }

    return env;
}
}
}
