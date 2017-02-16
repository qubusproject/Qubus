#include <qbb/qubus/IR/deduce_expression_environment.hpp>

#include <qbb/qubus/IR/qir.hpp>

#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <boost/range/adaptor/map.hpp>

#include <qbb/util/box.hpp>

#include <utility>
#include <map>
#include <vector>
#include <stack>

namespace qubus
{

namespace
{

void extract_expression_parameter(const expression& expr,
                                  std::map<util::handle, util::box<parameter>>& parameter_table,
                                  std::stack<std::vector<util::handle>>& scope_stack,
                                  bool is_modifing)
{
    using pattern::_;
    using pattern::value;

    pattern::variable<variable_declaration> var;
    pattern::variable<std::reference_wrapper<const expression>> scope;

    pattern::variable<std::reference_wrapper<const expression>> a, b;
    pattern::variable<std::vector<std::reference_wrapper<const expression>>> exprs;

    auto m =
        pattern::make_matcher<expression, void>()
            .case_(variable_scope(var, scope),
                   [&](const expression& self)
                   {
                       std::vector<util::handle> new_scope = {var.get().id()};

                       scope_stack.push(new_scope);

                       for (const auto& expr : self.sub_expressions())
                       {
                           extract_expression_parameter(expr, parameter_table, scope_stack, false);
                       }

                       for (const auto& entry : scope_stack.top())
                       {
                           parameter_table.erase(entry);
                       }

                       scope_stack.pop();
                   })
            .case_(local_variable_def(var, _),
                   [&]
                   {
                       scope_stack.top().push_back(var.get().id());
                   })
            .case_(binary_operator(
                       value(binary_op_tag::assign) || value(binary_op_tag::plus_assign), a, b),
                   [&]
                   {
                       extract_expression_parameter(a.get(), parameter_table, scope_stack, true);
                       extract_expression_parameter(b.get(), parameter_table, scope_stack, false);
                   })
            .case_(subscription(a, exprs),
                   [&]
                   {
                       extract_expression_parameter(a.get(), parameter_table, scope_stack,
                                                    is_modifing);

                       for (const auto& expr : exprs.get())
                       {
                           extract_expression_parameter(expr, parameter_table, scope_stack, false);
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
            .case_(spawn(_, exprs),
                   [&]
                   {
                       //TODO: Generalize this.
                       auto& exprs_ = exprs.get();

                       for (std::size_t i = 0; i < exprs_.size(); ++i)
                       {
                           if (i < exprs_.size() - 1)
                           {
                               extract_expression_parameter(exprs_[i], parameter_table, scope_stack,
                                                            false);
                           }
                           else
                           {
                               extract_expression_parameter(exprs_[i], parameter_table, scope_stack,
                                                            true);
                           }
                       }
                   })
            .case_(_, [&](const expression& self)
                   {
                       for (const expression& expr : self.sub_expressions())
                       {
                           extract_expression_parameter(expr, parameter_table, scope_stack,
                                                        is_modifing);
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
    std::stack<std::vector<util::handle>> scope_stack;

    scope_stack.push(std::vector<util::handle>());

    extract_expression_parameter(expr, parameter_table, scope_stack, false);

    for (const auto& entry : scope_stack.top())
    {
        parameter_table.erase(entry);
    }

    for (const auto& param : parameter_table | boost::adaptors::map_values)
    {
        env.add_parameter(*param);
    }

    return env;
}
}
