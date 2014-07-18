#include <qbb/kubus/IR/pretty_printer.hpp>

#include <qbb/kubus/IR/kir.hpp>
#include <qbb/util/multi_method.hpp>
#include <qbb/util/unique_name_generator.hpp>

#include <iostream>
#include <mutex>
#include <map>

namespace qbb
{
namespace kubus
{

namespace
{

class pretty_printer_context
{
public:
    const std::string& get_name_for_handle(const qbb::util::handle& h) const
    {
        auto iter = symbol_table_.find(h);
        
        if(iter != symbol_table_.end())
        {
            return iter->second;
        }
        else
        {
            iter = symbol_table_.emplace(h, name_generator.get()).first;
            
            return iter->second;
        }
    }
private:
    mutable std::map<qbb::util::handle, std::string> symbol_table_;
    qbb::util::unique_name_generator name_generator;
};

qbb::util::multi_method<void(const qbb::util::virtual_<expression>&, pretty_printer_context&)> print = {};

void print_double_lit(const double_literal_expr& expr, pretty_printer_context&)
{
    std::cout << expr.value();
}

void print_float_lit(const float_literal_expr& expr, pretty_printer_context&)
{
    std::cout << expr.value();
}

void print_integer_lit(const integer_literal_expr& expr, pretty_printer_context&)
{
    std::cout << expr.value();
}

void print_index(const index_expr& expr, pretty_printer_context&)
{
    std::cout << expr.id();
}

void print_binary_op(const binary_operator_expr& expr, pretty_printer_context& ctx)
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

    print(expr.left(), ctx);
    std::cout << " " << op << " ";
    print(expr.right(), ctx);
}

void print_unary_op(const unary_operator_expr& expr, pretty_printer_context& ctx)
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
    print(expr.arg(), ctx);
}

void print_sum(const sum_expr& expr, pretty_printer_context& ctx)
{
    std::cout << "sum( ";

    print(expr.body(), ctx);

    std::cout << ", ";

    for (const auto& index : expr.indices())
    {
        print(index, ctx);
        std::cout << ", ";
    }

    std::cout << ")";
}

void print_tensor(const tensor_access_expr& expr, pretty_printer_context& ctx)
{
    std::cout << ctx.get_name_for_handle(expr.variable()->data_handle());
}

void print_subscription(const subscription_expr& expr, pretty_printer_context& ctx)
{
    print(expr.indexed_expr(), ctx);

    std::cout << "[";

    for (const auto& index : expr.indices())
    {
        print(index, ctx);
        std::cout << ", ";
    }

    std::cout << "]";
}

void print_type_conversion(const type_conversion_expr& expr, pretty_printer_context& ctx)
{
    print(expr.arg(), ctx);
}

void print_for_all(const for_all_expr& expr, pretty_printer_context& ctx)
{
    std::cout << "for all ";
    print(expr.index(), ctx);
    std::cout << "\n{\n";
    print(expr.body(), ctx);
    std::cout << "\n}";
}

void print_for(const for_expr& expr, pretty_printer_context& ctx)
{
    std::cout << "for ";
    print(expr.index(), ctx);
    std::cout << " in [";
    print(expr.lower_bound(), ctx);
    std::cout << ", ";
    print(expr.upper_bound(), ctx);
    std::cout << "]";
    std::cout << "\n{\n";
    print(expr.body(), ctx);
    std::cout << "\n}";
}

void print_compound(const compound_expr& expr, pretty_printer_context& ctx)
{
    for(const auto& sub_expr : expr.body())
    {
        print(sub_expr, ctx);
        std::cout << " \n";
    }
}

void print_intrinsic_function(const intrinsic_function_expr& expr, pretty_printer_context& ctx)
{
    std::cout << expr.name() << "(";
    
    for(const auto& arg : expr.args())
    {
        print(arg, ctx);
        std::cout << ", ";
    }
    
    std::cout << ")";
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
    print.add_specialization(print_for_all);
    print.add_specialization(print_for);
    print.add_specialization(print_compound);
    print.add_specialization(print_intrinsic_function);
}

std::once_flag pretty_printer_init_flag = {};

}

void pretty_print(const expression& expr)
{
    std::call_once(pretty_printer_init_flag, init_pretty_printer);

    pretty_printer_context ctx;
    
    print(expr, ctx);
}
}
}