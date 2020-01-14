#ifndef QUBUS_IR_VISITOR_HPP
#define QUBUS_IR_VISITOR_HPP

#include <qubus/IR/qir.hpp>

namespace qubus
{

class visitor
{
public:
    visitor() = default;
    virtual ~visitor() noexcept = default;

    virtual bool handle_binary_operators(const binary_operator_expr& expr) = 0;
    virtual bool handle_unary_operators(const unary_operator_expr& expr) = 0;
    virtual bool handle_compound_expressions(const compound_expr& expr) = 0;
    virtual bool handle_construct_expressions(const construct_expr& expr) = 0;
    virtual bool handle_for_expressions(const for_expr& expr) = 0;
    virtual bool handle_if_expressions(const if_expr& expr) = 0;
    virtual bool handle_integer_ranges(const integer_range_expr& expr) = 0;
    virtual bool handle_integer_literals(const integer_literal_expr& expr) = 0;
    virtual bool handle_double_literals(const double_literal_expr& expr) = 0;
    virtual bool handle_float_literals(const float_literal_expr& expr) = 0;
    virtual bool handle_bool_literals(const bool_literal_expr& expr) = 0;
    virtual bool handle_local_variable_definitions(const local_variable_def_expr& expr) = 0;
    virtual bool handle_member_accesses(const member_access_expr& expr) = 0;
    virtual bool handle_subscriptions(const subscription_expr& expr) = 0;
    virtual bool handle_variable_references(const variable_ref_expr& expr) = 0;
    virtual bool handle_symbols(const symbol_expr& expr) = 0;

    virtual bool handle_functions(const function& func) = 0;
    virtual bool handle_types(const type& t) = 0;
    virtual bool handle_variable_declarations(const variable_declaration& var) = 0;

    virtual bool enter_lexical_scope(const expression& scope_body) = 0;
    virtual bool exit_lexical_scope(const expression& scope_body) = 0;
protected:
    visitor(const visitor&) = default;
    visitor(visitor&&) noexcept = default;

    visitor& operator=(const visitor&) = default;
    visitor& operator=(visitor&&) noexcept = default;
};

class non_exhaustive_visitor : public visitor
{
public:
    bool handle_binary_operators(const binary_operator_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_unary_operators(const unary_operator_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_compound_expressions(const compound_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_construct_expressions(const construct_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_for_expressions(const for_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_if_expressions(const if_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_integer_ranges(const integer_range_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_integer_literals(const integer_literal_expr& expr) noexcept override
    {
        return true;
    }
    bool handle_double_literals(const double_literal_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_float_literals(const float_literal_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_bool_literals(const bool_literal_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_local_variable_definitions(const local_variable_def_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_member_accesses(const member_access_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_subscriptions(const subscription_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_variable_references(const variable_ref_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_symbols(const symbol_expr& expr) noexcept override
    {
        return true;
    }

    bool handle_functions(const function& func) noexcept override
    {
        return true;
    }

    bool handle_types(const type& t) noexcept override
    {
        return true;
    }

    bool handle_variable_declarations(const variable_declaration& var) noexcept override
    {
        return true;
    }

    bool enter_lexical_scope(const expression& scope_body) noexcept override
    {
        return true;
    }

    bool exit_lexical_scope(const expression& scope_body) noexcept override
    {
        return true;
    }
};

}

#endif
