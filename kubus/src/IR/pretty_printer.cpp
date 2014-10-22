#include <qbb/kubus/IR/pretty_printer.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/IR.hpp>

#include <qbb/util/multi_method.hpp>
#include <qbb/util/unique_name_generator.hpp>

#include <iostream>
#include <mutex>
#include <map>
#include <cassert>

namespace qbb
{
namespace kubus
{

namespace
{

class pretty_printer_context
{
public:
    pretty_printer_context() = default;

    pretty_printer_context(const pretty_printer_context&) = delete;
    pretty_printer_context& operator=(const pretty_printer_context&) = delete;

    const std::string& get_name_for_handle(const qbb::util::handle& h) const
    {
        auto iter = symbol_table_.find(h);

        if (iter != symbol_table_.end())
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

qbb::util::multi_method<void(const qbb::util::virtual_<expression>&, pretty_printer_context&)>
    print = {};

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
    default:
        assert(false);
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
    default:
        assert(false);
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
    for (const auto& sub_expr : expr.body())
    {
        print(sub_expr, ctx);
        std::cout << " \n";
    }
}

void print_intrinsic_function(const intrinsic_function_expr& expr, pretty_printer_context& ctx)
{
    std::cout << expr.name() << "(";

    for (const auto& arg : expr.args())
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

const char* translate_binary_op_tag(binary_op_tag tag)
{
    switch (tag)
    {
    case binary_op_tag::plus:
        return "+";
    case binary_op_tag::minus:
        return "-";
    case binary_op_tag::multiplies:
        return "*";
    case binary_op_tag::divides:
        return "/";
    case binary_op_tag::assign:
        return "=";
    case binary_op_tag::plus_assign:
        return "+=";
    case binary_op_tag::equal_to:
        return "==";
    case binary_op_tag::not_equal_to:
        return "!=";
    case binary_op_tag::greater:
        return ">";
    case binary_op_tag::less:
        return "<";
    case binary_op_tag::greater_equal:
        return ">=";
    case binary_op_tag::less_equal:
        return "<=";
    case binary_op_tag::logical_and:
        return "&&";
    case binary_op_tag::logical_or:
        return "||";
    default:
        assert(false);
        break;
    }
}

const char* translate_unary_op_tag(unary_op_tag tag)
{
    switch (tag)
    {
    case unary_op_tag::plus:
        return "+";
    case unary_op_tag::negate:
        return "-";
    case unary_op_tag::logical_not:
        return "*";
    default:
        assert(false);
        break;
    }
}

void print_new(const expression& expr, pretty_printer_context& ctx)
{
    pattern::variable<expression> a, b, c, d;
    pattern::variable<binary_op_tag> btag;
    pattern::variable<unary_op_tag> utag;

    pattern::variable<std::vector<expression>> indices;
    pattern::variable<std::vector<expression>> subexprs;
    pattern::variable<std::vector<expression>> args;
    pattern::variable<std::string> id;

    pattern::variable<double> dval;
    pattern::variable<float> fval;
    pattern::variable<qbb::util::index_t> ival;

    pattern::variable<std::shared_ptr<tensor_variable>> tensor_var;

    auto m =
        pattern::make_matcher<expression, void>()
            .case_(binary_operator(btag, a, b), [&]
                   {
                print_new(a.get(), ctx);
                std::cout << " " << translate_binary_op_tag(btag.get()) << " ";
                print_new(b.get(), ctx);
            })
            .case_(unary_operator(utag, a), [&]
                   {
                std::cout << " " << translate_unary_op_tag(utag.get()) << " ";
                print_new(a.get(), ctx);
            })
            .case_(sum(a, indices), [&]
                   {
                std::cout << "sum( ";

                print_new(a.get(), ctx);

                std::cout << ", ";

                for (const auto& index : indices.get())
                {
                    print_new(index, ctx);
                    std::cout << ", ";
                }

                std::cout << ")";
            })
            .case_(index(id), [&]
                   {
                std::cout << id.get();
            })
            .case_(subscription(a, indices), [&]
                   {
                print_new(a.get(), ctx);

                std::cout << "[";

                for (const auto& index : indices.get())
                {
                    print_new(index, ctx);
                    std::cout << ", ";
                }

                std::cout << "]";
            })
            .case_(pattern::tensor(tensor_var), [&]
                   {
                std::cout << ctx.get_name_for_handle(tensor_var.get()->data_handle());
            })
            .case_(type_conversion(a), [&]
                   {
                print_new(a.get(), ctx);
            })
            .case_(compound(subexprs), [&]
                   {
                for (const auto& sub_expr : subexprs.get())
                {
                    print_new(sub_expr, ctx);
                    std::cout << " \n";
                }
            })
            .case_(intrinsic_function(id, args), [&]
                   {
                std::cout << id.get() << "(";

                for (const auto& arg : args.get())
                {
                    print_new(arg, ctx);
                    std::cout << ", ";
                }

                std::cout << ")";
            })
            .case_(double_literal(dval), [&]
                   {
                std::cout << dval.get();
            })
            .case_(float_literal(fval), [&]
                   {
                std::cout << fval.get();
            })
            .case_(integer_literal(ival), [&]
                   {
                std::cout << ival.get();
            })
            .case_(for_(a, b, c, d), [&]
                   {
                std::cout << "for ";
                print_new(a.get(), ctx);
                std::cout << " in [";
                print_new(b.get(), ctx);
                std::cout << ", ";
                print_new(c.get(), ctx);
                std::cout << "]";
                std::cout << "\n{\n";
                print_new(d.get(), ctx);
                std::cout << "\n}";
            })
            .case_(for_all(a, b), [&]
                   {
                std::cout << "for all ";
                print_new(a.get(), ctx);
                std::cout << "\n{\n";
                print_new(b.get(), ctx);
                std::cout << "\n}";
            });

    pattern::match(expr, m);
}
}

void pretty_print(const expression& expr)
{
    // std::call_once(pretty_printer_init_flag, init_pretty_printer);

    pretty_printer_context ctx;

    print_new(expr, ctx);
}
}
}
