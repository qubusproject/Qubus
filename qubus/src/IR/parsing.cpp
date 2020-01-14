#include <qubus/IR/parsing.hpp>

#include <qubus/IR/qir.hpp>

//#define BOOST_SPIRIT_X3_DEBUG
//
// #include <boost/optional/optional_io.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/optional.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/unreachable.hpp>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace qubus
{

namespace x3 = boost::spirit::x3;

namespace
{
namespace ast
{

struct basic_type;
struct parameterized_type;

struct type : x3::variant<x3::forward_ast<basic_type>, x3::forward_ast<parameterized_type>>
{
    // workaround for GCC <= 7
    type(const type&) = default;
    type(type&&) = default;

    type& operator=(const type&) = default;
    type& operator=(type&&) = default;

    using base_type::base_type;
    using base_type::operator=;
};

struct type_parameter : x3::variant<type, int>
{
    // workaround for GCC <= 7
    type_parameter(const type_parameter&) = default;
    type_parameter(type_parameter&&) = default;

    type_parameter& operator=(const type_parameter&) = default;
    type_parameter& operator=(type_parameter&&) = default;

    using base_type::base_type;
    using base_type::operator=;
};

struct basic_type : x3::position_tagged
{
    std::string name;
};

struct parameterized_type : x3::position_tagged
{
    std::string name;
    std::vector<type_parameter> parameters;
};

struct variable;
struct binary_operator;
struct unary_operator;
struct qualified_expr;
struct if_expr;
struct for_expr;
struct let_expr;
struct integer_range;
struct function_call;

struct expression : x3::variant<x3::forward_ast<variable>, x3::forward_ast<binary_operator>,
                                x3::forward_ast<unary_operator>, x3::forward_ast<qualified_expr>,
                                x3::forward_ast<if_expr>, x3::forward_ast<for_expr>,
                                x3::forward_ast<let_expr>, x3::forward_ast<integer_range>,
                                x3::forward_ast<function_call>, util::index_t, double, bool>
{
    // workaround for GCC <= 7
    expression(const expression&) = default;
    expression(expression&&) = default;

    expression& operator=(const expression&) = default;
    expression& operator=(expression&&) = default;

    using base_type::base_type;
    using base_type::operator=;
};

struct expression_block : x3::position_tagged
{
    std::vector<ast::expression> expressions;
    bool scoped;
};

struct variable : x3::position_tagged
{
    std::string name;
};

struct binary_operator : x3::position_tagged
{
    expression lhs;
    std::string operator_id;
    expression rhs;
};

struct unary_operator : x3::position_tagged
{
    std::string operator_id;
    expression arg;
};

struct subscription;
struct member_access;

struct qualifier : x3::variant<x3::forward_ast<subscription>, x3::forward_ast<member_access>>
{
    // workaround for GCC <= 7
    qualifier(const qualifier&) = default;
    qualifier(qualifier&&) = default;

    qualifier& operator=(const qualifier&) = default;
    qualifier& operator=(qualifier&&) = default;

    using base_type::base_type;
    using base_type::operator=;
};

struct qualified_expr : x3::position_tagged
{
    expression expr;
    std::vector<qualifier> qualifiers;
};

struct subscription : x3::position_tagged
{
    std::vector<expression> indices;
};

struct member_access : x3::position_tagged
{
    std::string member_name;
};

struct function_call : x3::position_tagged
{
    std::string name;
    std::vector<expression> arguments;
};

struct if_expr : x3::position_tagged
{
    expression condition;
    expression_block then_branch;
    boost::optional<expression_block> else_branch;
};

struct integer_range : x3::position_tagged
{
    expression lower_bound;
    expression stride;
    expression upper_bound;
};

struct variable_declaration : x3::position_tagged
{
    std::string name;
    type datatype;
};

struct for_expr : x3::position_tagged
{
    std::string ordering;
    variable_declaration loop_index;
    integer_range range;
    expression_block body;
};

struct let_expr : x3::position_tagged
{
    variable_declaration var;
    expression initializer;
};

struct template_parameter : x3::position_tagged
{
    std::string name;
    std::optional<type> datatype;
};

struct function : x3::position_tagged
{
    std::string id;
    std::vector<template_parameter> template_parameters;
    std::vector<variable_declaration> parameters;
    variable_declaration result;
    expression_block body;
};

struct struct_ : x3::position_tagged
{
    std::string name;
    std::vector<template_parameter> template_parameters;
    std::vector<variable_declaration> members;
};

struct definition : x3::variant<function, struct_>
{
    // workaround for GCC <= 7
    definition(const definition&) = default;
    definition(definition&&) = default;

    definition& operator=(const definition&) = default;
    definition& operator=(definition&&) = default;

    using base_type::base_type;
    using base_type::operator=;
};

struct module : x3::position_tagged
{
    std::string id;

    std::vector<definition> definitions;
};
} // namespace ast
} // namespace
} // namespace qubus

BOOST_FUSION_ADAPT_STRUCT(qubus::ast::basic_type, (std::string, name));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::parameterized_type,
                          (std::string, name)(std::vector<qubus::ast::type_parameter>, parameters));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::variable_declaration,
                          (std::string, name)(qubus::ast::type, datatype));

BOOST_FUSION_ADAPT_STRUCT(qubus::ast::expression_block,
                          (std::vector<qubus::ast::expression>, expressions)(bool, scoped));

BOOST_FUSION_ADAPT_STRUCT(qubus::ast::variable, (std::string, name));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::binary_operator,
                          (qubus::ast::expression, lhs)(std::string,
                                                        operator_id)(qubus::ast::expression, rhs));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::unary_operator,
                          (std::string, operator_id)(qubus::ast::expression, arg));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::subscription, (std::vector<qubus::ast::expression>, indices));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::member_access, (std::string, member_name));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::function_call,
                          (std::string, name)(std::vector<qubus::ast::expression>, arguments));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::qualified_expr,
                          (qubus::ast::expression, expr)(std::vector<qubus::ast::qualifier>,
                                                         qualifiers));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::if_expr,
                          (qubus::ast::expression,
                           condition)(qubus::ast::expression_block,
                                      then_branch)(boost::optional<qubus::ast::expression_block>,
                                                   else_branch));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::for_expr,
                          (std::string, ordering)(qubus::ast::variable_declaration,
                                                  loop_index)(qubus::ast::integer_range,
                                                              range)(qubus::ast::expression_block,
                                                                     body));

BOOST_FUSION_ADAPT_STRUCT(qubus::ast::let_expr, (qubus::ast::variable_declaration,
                                                 var)(qubus::ast::expression, initializer));

BOOST_FUSION_ADAPT_STRUCT(qubus::ast::integer_range,
                          (qubus::ast::expression, lower_bound)(qubus::ast::expression,
                                                                stride)(qubus::ast::expression,
                                                                        upper_bound));

BOOST_FUSION_ADAPT_STRUCT(qubus::ast::module,
                          (std::string, id)(std::vector<qubus::ast::definition>, definitions));

BOOST_FUSION_ADAPT_STRUCT(qubus::ast::template_parameter,
                          (std::string, name)
                          (std::optional<qubus::ast::type>, datatype));

BOOST_FUSION_ADAPT_STRUCT(qubus::ast::function,
                          (std::string, id)
                          (std::vector<qubus::ast::template_parameter>, template_parameters)
                          (std::vector<qubus::ast::variable_declaration>, parameters)
                          (qubus::ast::variable_declaration, result)
                          (qubus::ast::expression_block, body));
BOOST_FUSION_ADAPT_STRUCT(qubus::ast::struct_,
                          (std::string, name)
                          (std::vector<qubus::ast::template_parameter>, template_parameters)
                          (std::vector<qubus::ast::variable_declaration>, members));

namespace qubus
{
namespace
{
namespace grammar
{

x3::rule<class id, std::string> id = "id";

x3::rule<class basic_type, ast::basic_type> basic_type = "basic_type";
x3::rule<class parameterized_type, ast::parameterized_type> parameterized_type =
    "parameterized_type";
class type;
x3::rule<type, ast::type> type = "type";

class module;
x3::rule<module, ast::module> module = "module";

class definition;
x3::rule<definition, ast::definition> definition = "definition";
class function;
x3::rule<function, ast::function> function = "function";
x3::rule<class user_defined_type, ast::struct_> user_defined_type = "user_defined_type";
x3::rule<class struct_definition, ast::struct_> struct_definition = "struct_definition";
class variable_declaration;
x3::rule<variable_declaration, ast::variable_declaration> variable_declaration =
    "variable_declaration";
class template_parameter;
x3::rule<template_parameter, qubus::ast::template_parameter> template_parameter = "template_parameter";
x3::rule<class template_parameters, std::vector<qubus::ast::template_parameter>> template_parameters = "template_parameters";
x3::rule<class expression_block, ast::expression_block> expression_block = "expression_block";

class expression;
x3::rule<expression, ast::expression> expression = "expression";
x3::rule<class variable, ast::variable> variable = "variable";
x3::rule<class assignment, ast::binary_operator> assignment = "assignment";
x3::rule<class non_task_expr, ast::expression> non_task_expr = "non_task_expr";
x3::rule<class logical_expr, ast::binary_operator> logical_expr = "logical_expr";
x3::rule<class logical_expressions, ast::expression> logical_expressions = "logical_expressions";
x3::rule<class comparison, ast::binary_operator> comparison = "comparison";
x3::rule<class comparisons, ast::expression> comparisons = "comparisons";
x3::rule<class addition, ast::binary_operator> addition = "addition";
x3::rule<class additions, ast::expression> additions = "additions";
x3::rule<class multiplication, ast::binary_operator> multiplication = "multiplication";
x3::rule<class multiplications, ast::expression> multiplications = "multiplications";
x3::rule<class qualified_expr, ast::qualified_expr> qualified_expr = "qualified_expr";
x3::rule<class base_expression, ast::expression> base_expression = "base_expression";
x3::rule<class function_call, ast::function_call> function_call = "function_call";
x3::rule<class unary_operator, ast::unary_operator> unary_operator = "unary_operator";
x3::rule<class unary_operators, ast::expression> unary_operators = "unary_operators";
x3::rule<class subscription, ast::subscription> subscription = "subscription";
x3::rule<class member_access, ast::member_access> member_access = "member_access";
class if_expr;
x3::rule<if_expr, ast::if_expr> if_expr = "if_expr";
class for_expr;
x3::rule<for_expr, ast::for_expr> for_expr = "for_expr";
x3::rule<class let_expr, ast::let_expr> let_expr = "let_expr";

x3::rule<class integer_range, ast::integer_range> integer_range = "integer_range";

const auto keyword = x3::lit("for") | x3::lit("if") | x3::lit("else") | x3::lit("end") |
                     x3::lit("module") | x3::lit("struct") | x3::lit("function") | x3::lit("let") |
                     x3::lit("parallel") | x3::lit("unordered") | x3::lit("true") |
                     x3::lit("false") | x3::lit("builtin");

const auto id_def = x3::raw[x3::lexeme[x3::ascii::alpha >> *(x3::ascii::alnum | '_')]] - keyword;
const auto boolean = "true" >> x3::attr(true) | "false" >> x3::attr(false);
const auto strict_double = x3::real_parser<double, x3::strict_real_policies<double>>();
const auto literal = strict_double | x3::long_ | boolean;
const auto integer_range_def = logical_expressions >> ':' >>
                               (logical_expressions >> ':' | x3::attr(1l)) >> logical_expressions;

const auto type_parameter = type | x3::long_;
const auto basic_type_def = id;
const auto parameterized_type_def = id >> ('{' > -(type_parameter % ',') > '}');
const auto type_def = parameterized_type | basic_type;

const auto variable_declaration_def = id >> ("::" > type);

const auto template_parameter_def = id >> -("::" > type);
const auto template_parameters_def = '{' > -(template_parameter % ',') > '}';

const auto module_def = "module" > id > *definition;
const auto definition_def = function | user_defined_type;
const auto function_def = "function" > id > -template_parameters > '(' > -(variable_declaration % ',') > ')' >
                          "->" > variable_declaration > expression_block > "end";
const auto user_defined_type_def = struct_definition;
const auto struct_definition_def = "struct" > id > -template_parameters > *variable_declaration > "end";

const auto expression_block_def = *expression >> x3::attr(false);
const auto expression_def = if_expr | for_expr | let_expr | assignment;
const auto variable_def = id;

const auto multiplication_op = x3::string("*") | x3::string("/") | x3::string("%");
const auto multiplication_def = unary_operators >> multiplication_op >> multiplications;
const auto multiplications_def = multiplication | unary_operators;

const auto addition_op = x3::string("+") | x3::string("-");
const auto addition_def = multiplications >> addition_op >> additions;
const auto additions_def = addition | multiplications;

const auto comparison_op = x3::string("==") | x3::string("!=") | x3::string("<") | x3::string(">") |
                           x3::string("<=") | x3::string(">=");
const auto comparison_def = additions >> comparison_op >> comparisons;
const auto comparisons_def = comparison | additions;

const auto logical_op = x3::string("and") | x3::string("or");
const auto logical_expr_def = comparisons >> logical_op >> logical_expressions;
const auto logical_expressions_def = logical_expr | comparisons;

const auto non_task_expr_def = integer_range | logical_expressions;

const auto assignment_op = x3::string("=") | x3::string("+=");
const auto assignment_def = non_task_expr >> assignment_op > non_task_expr;

const auto unary_op = x3::string("+") | x3::string("-") | x3::string("not");
const auto unary_operator_def = unary_op >> unary_operators;
const auto unary_operators_def = unary_operator | qualified_expr;

const auto subscription_def = '[' >> -(non_task_expr % ',') >> ']';
const auto member_access_def = '.' >> id;

const auto qualified_expr_def = base_expression >> *(subscription | member_access);

const auto function_call_def = id >> '(' >> non_task_expr % ',' >> ')';

const auto base_expression_def = '(' >> non_task_expr >> ')' | function_call | variable | literal;

const auto if_expr_def = "if" > non_task_expr > expression_block > -("else" > expression_block) >
                         "end";

const auto ordering =
    x3::string("parallel") | x3::string("unordered") | x3::attr(std::string("sequential"));
const auto for_expr_def =
    ordering >> "for" > variable_declaration > "in" > integer_range > expression_block > "end";

const auto let_expr_def = "let" > variable_declaration > '=' > non_task_expr;

const auto skipper = x3::ascii::space;

BOOST_SPIRIT_DEFINE(id, module, definition, function, expression_block, user_defined_type,
                    struct_definition, variable_declaration, template_parameters, template_parameter,
                    type, parameterized_type,
                    basic_type, expression, variable, assignment, non_task_expr, logical_expr,
                    logical_expressions, comparison, comparisons, addition, additions,
                    multiplication, multiplications, qualified_expr, base_expression,
                    unary_operator, unary_operators, subscription, member_access, if_expr, for_expr,
                    integer_range, let_expr, function_call);
} // namespace grammar

class type_analyzer
{
public:
    using result_type = type;

    result_type operator()(const ast::parameterized_type& pt) const
    {
        if (pt.name == "Array")
        {
            if (pt.parameters.size() != 2)
                throw parsing_error("Invalid number of type parameters.");

            auto value_type = boost::get<ast::type>(&pt.parameters[0]);
            auto rank = boost::get<int>(&pt.parameters[1]);

            if (value_type && rank)
            {
                return types::array(value_type->apply_visitor(*this), *rank);
            }
            else
            {
                throw parsing_error("Invalid parameters type.");
            }
        }
        else if (pt.name == "Complex")
        {
            if (pt.parameters.size() != 1)
                throw parsing_error("Invalid number of type parameters.");

            auto value_type = boost::get<ast::type>(&pt.parameters[0]);

            if (value_type)
            {
                return types::complex(value_type->apply_visitor(*this));
            }
            else
            {
                throw parsing_error("Invalid parameters type.");
            }
        }
        else
        {
            throw parsing_error("Unknown type.");
        }
    }

    result_type operator()(const ast::basic_type& bt) const
    {
        if (bt.name == "Int")
        {
            return types::integer();
        }
        else if (bt.name == "Double")
        {
            return types::double_();
        }
        else if (bt.name == "Float")
        {
            return types::float_();
        }
        else if (bt.name == "Bool")
        {
            return types::bool_();
        }
        else
        {
            throw parsing_error("Unknown type.");
        }
    }
};

class symbol_scope
{
public:
    std::shared_ptr<const variable_declaration> create_variable(const std::string& name, const ast::type& datatype)
    {
        auto ir_datatype = datatype.apply_visitor(type_analyzer());

        auto var = std::make_shared<variable_declaration>(name, std::move(ir_datatype));

        auto [iter, inserted] = symbol_table_.emplace(name, std::move(var));

        if (!inserted)
            throw parsing_error("Duplicate definition.");

        return std::static_pointer_cast<const variable_declaration>(iter->second);
    }

    std::shared_ptr<const variable_declaration> create_variable(const ast::variable_declaration& decl)
    {
        return create_variable(decl.name, decl.datatype);
    }

    std::shared_ptr<const variable_declaration> lookup_variable(const ast::variable& var) const
    {
        auto search_result = symbol_table_.find(var.name);

        if (search_result == symbol_table_.end())
            return nullptr;

        if (auto var = std::dynamic_pointer_cast<const variable_declaration>(search_result->second))
        {
            return var;
        }
        else
        {
            throw parsing_error("The symbol is of the wrong type.");
        }
    }

    std::shared_ptr<const symbolic_type> create_symbolic_type(const std::string& name)
    {

    }

    std::shared_ptr<const template_parameter> create_template_parameter(const ast::template_parameter& template_param)
    {
        if (template_param.datatype)
        {
            return create_variable(template_param.name, *template_param.datatype);
        }
        else
        {
            // return create_symbolic_type(template_param.name);
        }
    }
private:
    using symbol_data_type = std::shared_ptr<const definition>;

    std::unordered_map<std::string, symbol_data_type> symbol_table_;
};

class symbol_table
{
public:
    symbol_table()
    {
        enter_scope();
    }

    std::shared_ptr<const variable_declaration> create_variable(const ast::variable_declaration& decl)
    {
        return scopes_.back().create_variable(decl);
    }

    std::shared_ptr<const variable_declaration> lookup_variable(const ast::variable& var) const
    {
        for (auto iter = scopes_.rbegin(), end = scopes_.rend(); iter != end; ++iter)
        {
            if (auto result = iter->lookup_variable(var))
            {
                return result;
            }
        }

        throw parsing_error("Unknown variable.");
    }

    std::shared_ptr<const symbolic_type> create_symbolic_type(const std::string& name)
    {
        return scopes_.back().create_symbolic_type(name);
    }

    std::shared_ptr<const template_parameter> create_template_parameter(const ast::template_parameter& template_param)
    {
        return scopes_.back().create_template_parameter(template_param);
    }

    void enter_scope()
    {
        scopes_.push_back(symbol_scope());
    }

    void leave_scope()
    {
        scopes_.pop_back();
    }

private:
    std::vector<symbol_scope> scopes_;
};

class scope_guard
{
public:
    explicit scope_guard(symbol_table& sym_table_) : sym_table_(&sym_table_)
    {
        sym_table_.enter_scope();
    }

    ~scope_guard()
    {
        if (sym_table_)
        {
            sym_table_->leave_scope();
        }
    }

    scope_guard(const scope_guard&) = delete;
    scope_guard(scope_guard&&) = delete;

    scope_guard& operator=(const scope_guard&) = delete;
    scope_guard& operator=(scope_guard&&) = delete;

private:
    symbol_table* sym_table_;
};

class expression_analyzer;

std::unique_ptr<expression> analyze_expression_block(const ast::expression_block& block,
                                                     expression_analyzer& analyzer);

class qualifier_analyzer
{
public:
    using result_type = std::unique_ptr<access_expr>;

    qualifier_analyzer(expression_analyzer& expr_analyzer_, std::unique_ptr<access_expr> arg_)
    : expr_analyzer_(&expr_analyzer_), arg_(std::move(arg_))
    {
    }

    result_type operator()(const ast::subscription& sub);
    result_type operator()(const ast::member_access& member_acc);

private:
    expression_analyzer* expr_analyzer_;
    std::unique_ptr<access_expr> arg_;
};

class expression_analyzer
{
public:
    using result_type = std::unique_ptr<expression>;

    explicit expression_analyzer(symbol_table& sym_table_) : sym_table_(&sym_table_)
    {
    }

    result_type operator()(const ast::variable& var)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        return variable_ref(sym_table_->lookup_variable(var));
    }

    result_type operator()(const ast::binary_operator& op)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        auto lhs = op.lhs.apply_visitor(*this);
        auto rhs = op.rhs.apply_visitor(*this);

        if (op.operator_id == "=")
        {
            return binary_operator(binary_op_tag::assign, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "+=")
        {
            return binary_operator(binary_op_tag::plus_assign, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "+")
        {
            return binary_operator(binary_op_tag::plus, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "-")
        {
            return binary_operator(binary_op_tag::minus, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "*")
        {
            return binary_operator(binary_op_tag::multiplies, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "/")
        {
            return binary_operator(binary_op_tag::divides, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "%")
        {
            return binary_operator(binary_op_tag::modulus, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "==")
        {
            return binary_operator(binary_op_tag::equal_to, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "!=")
        {
            return binary_operator(binary_op_tag::not_equal_to, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "<")
        {
            return binary_operator(binary_op_tag::less, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "<=")
        {
            return binary_operator(binary_op_tag::less_equal, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == ">")
        {
            return binary_operator(binary_op_tag::greater, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == ">=")
        {
            return binary_operator(binary_op_tag::greater_equal, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "and")
        {
            return binary_operator(binary_op_tag::logical_and, std::move(lhs), std::move(rhs));
        }
        else if (op.operator_id == "or")
        {
            return binary_operator(binary_op_tag::logical_or, std::move(lhs), std::move(rhs));
        }

        QUBUS_UNREACHABLE();
    }

    result_type operator()(const ast::unary_operator& op)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        auto arg = op.arg.apply_visitor(*this);

        if (op.operator_id == "+")
        {
            return unary_operator(unary_op_tag::plus, std::move(arg));
        }
        else if (op.operator_id == "-")
        {
            return unary_operator(unary_op_tag::negate, std::move(arg));
        }
        else if (op.operator_id == "not")
        {
            return unary_operator(unary_op_tag::logical_not, std::move(arg));
        }

        QUBUS_UNREACHABLE();
    }

    result_type operator()(const ast::qualified_expr& expr)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        auto arg = expr.expr.apply_visitor(*this);

        if (expr.qualifiers.empty())
            return arg;

        if (auto arg_acc = dynamic_cast<access_expr*>(arg.get()))
        {
            arg.release();
            auto arg_ = std::unique_ptr<access_expr>(arg_acc);

            for (const auto& qualifier : expr.qualifiers)
            {
                qualifier_analyzer analyzer(*this, std::move(arg_));
                arg_ = qualifier.apply_visitor(analyzer);
            }

            return arg_;
        }

        throw parsing_error("Invalid expression.");
    }

    result_type operator()(const ast::if_expr& expr)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        scope_guard guard(*sym_table_);

        auto cond = expr.condition.apply_visitor(*this);
        auto then_branch = analyze_expression_block(expr.then_branch, *this);

        if (expr.else_branch)
        {
            auto else_branch = analyze_expression_block(*expr.else_branch, *this);

            return if_(std::move(cond), std::move(then_branch), std::move(else_branch));
        }
        else
        {
            return if_(std::move(cond), std::move(then_branch));
        }
    }

    result_type operator()(const ast::for_expr& expr)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        scope_guard guard(*sym_table_);

        auto loop_index = sym_table_->create_variable(expr.loop_index);

        auto lower_bound = expr.range.lower_bound.apply_visitor(*this);
        auto upper_bound = expr.range.upper_bound.apply_visitor(*this);
        auto increment = expr.range.stride.apply_visitor(*this);

        auto body = analyze_expression_block(expr.body, *this);

        if (expr.ordering == "parallel")
            return parallel_for(std::move(loop_index), std::move(lower_bound),
                                std::move(upper_bound), std::move(increment), std::move(body));

        if (expr.ordering == "unordered")
            return unordered_for(std::move(loop_index), std::move(lower_bound),
                                 std::move(upper_bound), std::move(increment), std::move(body));

        if (expr.ordering == "sequential")
            return for_(std::move(loop_index), std::move(lower_bound), std::move(upper_bound),
                        std::move(increment), std::move(body));

        QUBUS_UNREACHABLE_BECAUSE("No other ordering exists.");
    }

    result_type operator()(const ast::let_expr& expr)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        auto decl = sym_table_->create_variable(expr.var);
        auto initializer = expr.initializer.apply_visitor(*this);

        return local_variable_def(std::move(decl), std::move(initializer));
    }

    result_type operator()(const ast::function_call& expr)
    {
        std::vector<std::unique_ptr<expression>> arguments;
        arguments.reserve(expr.arguments.size());

        for (const auto& arg : expr.arguments)
        {
            arguments.push_back(arg.apply_visitor(*this));
        }

        return intrinsic_function(expr.name, std::move(arguments));
    }

    result_type operator()(util::index_t value)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        return integer_literal(value);
    }

    result_type operator()(double value)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        return lit(value);
    }

    result_type operator()(bool value)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        return lit(value);
    }

    result_type operator()(const ast::integer_range& expr)
    {
        QUBUS_ASSERT(sym_table_ != nullptr, "expression_analyzer has not been initialized.");

        auto lower_bound = expr.lower_bound.apply_visitor(*this);
        auto upper_bound = expr.upper_bound.apply_visitor(*this);
        auto stride = expr.stride.apply_visitor(*this);

        return range(std::move(lower_bound), std::move(upper_bound), std::move(stride));
    }

private:
    symbol_table* sym_table_;
};

qualifier_analyzer::result_type qualifier_analyzer::operator()(const ast::subscription& sub)
{
    QUBUS_ASSERT(expr_analyzer_ != nullptr, "qualifier_analyzer has not been initialized.");

    std::vector<std::unique_ptr<expression>> indices;

    for (const auto& index : sub.indices)
    {
        indices.push_back(index.apply_visitor(*expr_analyzer_));
    }

    return subscription(std::move(arg_), std::move(indices));
}

qualifier_analyzer::result_type qualifier_analyzer::operator()(const ast::member_access& member_acc)
{
    QUBUS_ASSERT(expr_analyzer_ != nullptr, "qualifier_analyzer has not been initialized.");

    return member_access(std::move(arg_), member_acc.member_name);
}

std::unique_ptr<expression> analyze_expression_block(const ast::expression_block& block,
                                                     expression_analyzer& analyzer)
{
    std::vector<std::unique_ptr<expression>> expressions;

    for (const auto& expr : block.expressions)
    {
        expressions.push_back(expr.apply_visitor(analyzer));
    }

    return sequenced_tasks(std::move(expressions));
}

class definition_analyzer
{
public:
    using result_type = void;

    explicit definition_analyzer(module& mod_) : mod_(&mod_)
    {
    }

    result_type operator()(const ast::function& func)
    {
        QUBUS_ASSERT(mod_ != nullptr, "definition_analyzer has not been initialized.");

        scope_guard guard(sym_table_);

        std::vector<std::shared_ptr<const template_parameter>> template_params;

        for (const auto& template_param : func.template_parameters)
        {
            template_params.push_back(sym_table_.create_template_parameter(template_param));
        }

        std::vector<std::shared_ptr<const variable_declaration>> params;

        for (const auto& param : func.parameters)
        {
            params.push_back(sym_table_.create_variable(param));
        }

        std::shared_ptr<const variable_declaration> result = sym_table_.create_variable(func.result);

        expression_analyzer expr_analyzer(sym_table_);

        auto body = analyze_expression_block(func.body, expr_analyzer);

        if (params.empty())
        {
            mod_->add_function(func.id, std::move(params), std::move(result), std::move(body));
        }
        else
        {
            // TODO: Add a function template to the module.
        }
    }

    result_type operator()(const ast::struct_& s)
    {
        QUBUS_ASSERT(mod_ != nullptr, "definition_analyzer has not been initialized.");

        std::vector<types::struct_::member> members;

        for (const auto& member : s.members)
        {
            auto datatype = member.datatype.apply_visitor(type_analyzer());

            members.emplace_back(std::move(datatype), member.name);
        }

        types::struct_ ss(s.name, std::move(members), {});

        mod_->add_type(std::move(ss));
    }

private:
    symbol_table sym_table_;
    module* mod_;
};

bool isascii(char value)
{
    static_assert(sizeof(value) == sizeof(std::uint8_t));

    std::uint8_t byte;

    std::memcpy(&byte, &value, sizeof(std::uint8_t));

    return ((byte >> 7) & uint8_t(1)) == 0;
}
} // namespace

std::unique_ptr<module> parse_qir(const std::string& code)
{
    auto first = code.cbegin();
    auto last = code.cend();

    auto contains_invalid_char =
        std::any_of(first, last, [](char value) { return !isascii(value); });

    if (contains_invalid_char)
        throw parsing_error("Invalid character.");

    ast::module ast;

    try
    {
        bool r = x3::phrase_parse(first, last, grammar::module, grammar::skipper, ast);

        if (!r || first != last)
            throw parsing_error("Syntax error.");
    }
    catch (const x3::expectation_failure<std::string::const_iterator>& e)
    {
        throw parsing_error("Syntax error.");
    }

    auto mod = std::make_unique<module>(symbol_id(ast.id));

    definition_analyzer def_analyzer(*mod);

    for (const auto& definition : ast.definitions)
    {
        definition.apply_visitor(def_analyzer);
    }

    return mod;
}
} // namespace qubus
