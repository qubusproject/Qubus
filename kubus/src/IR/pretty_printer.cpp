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

void print(const expression& expr, pretty_printer_context& ctx)
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
                print(a.get(), ctx);
                std::cout << " " << translate_binary_op_tag(btag.get()) << " ";
                print(b.get(), ctx);
            })
            .case_(unary_operator(utag, a), [&]
                   {
                std::cout << " " << translate_unary_op_tag(utag.get()) << " ";
                print(a.get(), ctx);
            })
            .case_(sum(a, indices), [&]
                   {
                std::cout << "sum( ";

                print(a.get(), ctx);

                std::cout << ", ";

                for (const auto& index : indices.get())
                {
                    print(index, ctx);
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
                print(a.get(), ctx);

                std::cout << "[";

                for (const auto& index : indices.get())
                {
                    print(index, ctx);
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
                print(a.get(), ctx);
            })
            .case_(compound(subexprs), [&]
                   {
                for (const auto& sub_expr : subexprs.get())
                {
                    print(sub_expr, ctx);
                    std::cout << " \n";
                }
            })
            .case_(intrinsic_function(id, args), [&]
                   {
                std::cout << id.get() << "(";

                for (const auto& arg : args.get())
                {
                    print(arg, ctx);
                    std::cout << ", ";
                }

                std::cout << ")";
            })
            .case_(delta(a, b), [&]
                   {
                std::cout << id.get() << "[";


                print(a.get(), ctx);
                std::cout << ", ";
                print(b.get(), ctx);

                std::cout << "]";
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
                print(a.get(), ctx);
                std::cout << " in [";
                print(b.get(), ctx);
                std::cout << ", ";
                print(c.get(), ctx);
                std::cout << "]";
                std::cout << "\n{\n";
                print(d.get(), ctx);
                std::cout << "\n}";
            })
            .case_(for_all(a, b), [&]
                   {
                std::cout << "for all ";
                print(a.get(), ctx);
                std::cout << "\n{\n";
                print(b.get(), ctx);
                std::cout << "\n}";
            });

    pattern::match(expr, m);
}
}

void pretty_print(const expression& expr)
{
    pretty_printer_context ctx;

    print(expr, ctx);
}
}
}
