#include <qbb/kubus/IR/pretty_printer.hpp>

#include <qbb/kubus/IR/kir.hpp>
#include <qbb/util/multi_method.hpp>

#include <iostream>
#include <mutex>

namespace qbb
{
namespace kubus
{

namespace
{

qbb::util::multi_method<void(const qbb::util::virtual_<expression>&)> print = {};

void print_double_lit(const double_literal_expr& expr)
{
    std::cout << expr.value();
}

void print_float_lit(const float_literal_expr& expr)
{
    std::cout << expr.value();
}

void print_integer_lit(const integer_literal_expr& expr)
{
    std::cout << expr.value();
}

void print_index(const index_expr& expr)
{
    std::cout << expr.id();
}

void print_binary_op(const binary_operator_expr& expr)
{
    std::string op;

    switch (expr.tag())
    {
    case binary_op_tag::plus:
        op = "+";
        break;
    case binary_op_tag::minus:
        op = "-";
        break;
    case binary_op_tag::multiplies:
        op = "*";
        break;
    case binary_op_tag::divides:
        op = "/";
        break;
    case binary_op_tag::assign:
        op = "=";
        break;
    case binary_op_tag::plus_assign:
        op = "+=";
        break;
    case binary_op_tag::equal_to:
        op = "==";
        break;
    case binary_op_tag::not_equal_to:
        op = "!=";
        break;
    case binary_op_tag::greater:
        op = ">";
        break;
    case binary_op_tag::less:
        op = "<";
        break;
    case binary_op_tag::greater_equal:
        op = ">=";
        break;
    case binary_op_tag::less_equal:
        op = "<=";
        break;
    case binary_op_tag::logical_and:
        op = "&&";
        break;
    case binary_op_tag::logical_or:
        op = "||";
        break;
    }

    print(expr.left());
    std::cout << " " << op << " ";
    print(expr.right());
}

void print_unary_op(const unary_operator_expr& expr)
{
    std::string op;

    switch (expr.tag())
    {
    case unary_op_tag::plus:
        op = "+";
        break;
    case unary_op_tag::negate:
        op = "-";
        break;
    case unary_op_tag::logical_not:
        op = "*";
        break;
    }

    std::cout << " " << op << " ";
    print(expr.arg());
}

void print_sum(const sum_expr& expr)
{
    std::cout << "sum( ";

    print(expr.body());

    std::cout << ", ";

    for (const auto& index : expr.indices())
    {
        print(index);
        std::cout << ", ";
    }

    std::cout << "]";
}

void print_tensor(const tensor_variable_expr&)
{
    std::cout << "tensor";
}

void print_subscription(const subscription_expr& expr)
{
    print(expr.indexed_expr());

    std::cout << "[";

    for (const auto& index : expr.indices())
    {
        print(index);
        std::cout << ", ";
    }

    std::cout << "]";
}

void print_type_conversion(const type_conversion_expr& expr)
{
    print(expr.arg());
}

void print_for(const for_expr& expr)
{
    std::cout << "for( ";
    print(expr.index());
    std::cout << ", ";
    print(expr.body());
    std::cout << " )";
}

void init_pretty_printer()
{
    print.add_specialization(print_double_lit);
    print.add_specialization(print_float_lit);
    print.add_specialization(print_integer_lit);
    print.add_specialization(print_index);
    print.add_specialization(print_binary_op);
    print.add_specialization(print_unary_op);
    print.add_specialization(print_sum);
    print.add_specialization(print_tensor);
    print.add_specialization(print_subscription);
    print.add_specialization(print_type_conversion);
    print.add_specialization(print_for);
}

std::once_flag pretty_printer_init_flag = {};

}

void pretty_print(const expression& expr)
{
    std::call_once(pretty_printer_init_flag, init_pretty_printer);

    print(expr);
}
}
}