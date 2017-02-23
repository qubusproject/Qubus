#ifndef QUBUS_DEDUCE_EXPRESSION_ENVIRONMENT_HPP
#define QUBUS_DEDUCE_EXPRESSION_ENVIRONMENT_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/variable_declaration.hpp>

#include <vector>

namespace qubus
{

class parameter
{
public:
    parameter(variable_intent intent_, variable_declaration declaration_)
    : intent_(intent_), declaration_(declaration_)
    {
    }

    variable_intent intent() const
    {
        return intent_;
    }

    variable_declaration declaration() const
    {
        return declaration_;
    }
private:
    variable_intent intent_;
    variable_declaration declaration_;
};

class expression_environment
{
public:
    void add_parameter(parameter p);

    const std::vector<parameter>& parameters() const;
private:
    std::vector<parameter> expression_parameters_;
};

expression_environment deduce_expression_environment(const expression& expr);
    
}

#endif