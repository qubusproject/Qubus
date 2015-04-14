#include <qbb/kubus/IR/pretty_printer.hpp>

#include <qbb/kubus/IR/type_inference.hpp>
#include <qbb/kubus/IR/kir.hpp>

#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/IR.hpp>

#include <qbb/util/multi_method.hpp>
#include <qbb/util/unique_name_generator.hpp>
#include <qbb/util/handle.hpp>

#include <qbb/util/unreachable.hpp>
#include <qbb/util/assert.hpp>

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
    case binary_op_tag::modulus:
        return "%";
    case binary_op_tag::div_floor:
        return "//";
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
        QBB_ASSERT(false, "default case should never be reached");
        QBB_UNREACHABLE();
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
        QBB_ASSERT(false, "default case should never be reached");
        QBB_UNREACHABLE();
    }
}

void print_type(const type& t)
{
    pattern::variable<type> subtype;

    auto m = pattern::make_matcher<type, void>()
                 .case_(pattern::double_t,
                        [&]
                        {
                            std::cout << "double";
                        })
                 .case_(pattern::integer_t,
                        [&]
                        {
                            std::cout << "integer";
                        })
                 .case_(complex_t(subtype),
                        [&]
                        {
                            std::cout << "complex<";

                            print_type(subtype.get());

                            std::cout << ">";
                        })
                 .case_(tensor_t(subtype),
                        [&]
                        {
                            std::cout << "tensor<";

                            print_type(subtype.get());

                            std::cout << ">";
                        })
                 .case_(pattern::_, [&]
                        {
                        });

    pattern::match(t, m);
}

void print(const expression& expr, pretty_printer_context& ctx, bool print_types)
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

    pattern::variable<variable_declaration> decl;
    pattern::variable<type> t;

    auto m =
        pattern::make_matcher<expression, void>()
            .case_(binary_operator(btag, a, b),
                   [&]
                   {
                       std::cout << "(";
                       print(a.get(), ctx, print_types);
                       std::cout << " " << translate_binary_op_tag(btag.get()) << " ";
                       print(b.get(), ctx, print_types);
                       std::cout << ")";
                   })
            .case_(unary_operator(utag, a),
                   [&]
                   {
                       std::cout << " " << translate_unary_op_tag(utag.get());
                       print(a.get(), ctx, print_types);
                   })
            .case_(sum(a, decl),
                   [&]
                   {
                       std::cout << "sum(";

                       print(a.get(), ctx, print_types);

                       std::cout << ", ";

                       if (auto debug_name = decl.get().annotations().lookup("kubus.debug.name"))
                       {
                           std::cout << debug_name.as<std::string>();
                       }
                       else
                       {
                           std::cout << ctx.get_name_for_handle(decl.get().id());
                       }

                       std::cout << ")";
                   })
            .case_(subscription(a, indices),
                   [&]
                   {
                       print(a.get(), ctx, print_types);

                       std::cout << "[";

                       for (const auto& index : indices.get())
                       {
                           print(index, ctx, print_types);
                           std::cout << ", ";
                       }

                       std::cout << "]";
                   })
            .case_(variable_ref(decl),
                   [&]
                   {
                       if (auto debug_name = decl.get().annotations().lookup("kubus.debug.name"))
                       {
                           std::cout << debug_name.as<std::string>();
                       }
                       else
                       {
                           std::cout << ctx.get_name_for_handle(decl.get().id());
                       }

                       if (print_types)
                       {
                           std::cout << " :: ";

                           print_type(decl.get().var_type());
                       }
                   })
            .case_(type_conversion(t, a),
                   [&]
                   {
                       std::cout << "cast<";
                       print_type(t.get());
                       std::cout << ">(";
                       print(a.get(), ctx, print_types);
                       std::cout << ")";
                   })
            .case_(compound(subexprs),
                   [&]
                   {
                       for (const auto& sub_expr : subexprs.get())
                       {
                           print(sub_expr, ctx, print_types);
                           std::cout << " \n";
                       }
                   })
            .case_(delta(a, b),
                   [&]
                   {
                       std::cout << id.get() << "[";

                       print(a.get(), ctx, print_types);
                       std::cout << ", ";
                       print(b.get(), ctx, print_types);

                       std::cout << "]";
                   })
            .case_(intrinsic_function(id, args),
                   [&]
                   {
                       std::cout << id.get() << "(";

                       for (const auto& arg : args.get())
                       {
                           print(arg, ctx, print_types);
                           std::cout << ", ";
                       }

                       std::cout << ")";
                   })
            .case_(double_literal(dval),
                   [&]
                   {
                       std::cout << dval.get();

                       if (print_types)
                       {
                           std::cout << " :: ";
                           print_type(types::double_{});
                       }
                   })
            .case_(float_literal(fval),
                   [&]
                   {
                       std::cout << fval.get();

                       if (print_types)
                       {
                           std::cout << " :: ";
                           print_type(types::float_{});
                       }
                   })
            .case_(integer_literal(ival),
                   [&]
                   {
                       std::cout << ival.get();

                       if (print_types)
                       {
                           std::cout << " :: ";
                           print_type(types::integer{});
                       }
                   })
            .case_(for_(decl, a, b, c, d),
                   [&]
                   {
                       std::cout << "for ";

                       if (auto debug_name = decl.get().annotations().lookup("kubus.debug.name"))
                       {
                           std::cout << debug_name.as<std::string>();
                       }
                       else
                       {
                           std::cout << ctx.get_name_for_handle(decl.get().id());
                       }

                       std::cout << " in [";
                       print(a.get(), ctx, print_types);
                       std::cout << ", ";
                       print(b.get(), ctx, print_types);
                       std::cout << ", ";
                       print(c.get(), ctx, print_types);
                       std::cout << "]";
                       std::cout << "\n{\n";
                       print(d.get(), ctx, print_types);
                       std::cout << "\n}";
                   })
            .case_(for_all(decl, b),
                   [&]
                   {
                       std::cout << "for all ";

                       if (auto debug_name = decl.get().annotations().lookup("kubus.debug.name"))
                       {
                           std::cout << debug_name.as<std::string>();
                       }
                       else
                       {
                           std::cout << ctx.get_name_for_handle(decl.get().id());
                       }

                       std::cout << "\n{\n";
                       print(b.get(), ctx, print_types);
                       std::cout << "\n}";
                   });

    pattern::match(expr, m);
}
}

void pretty_print(const expression& expr, bool print_types)
{
    pretty_printer_context ctx;

    print(expr, ctx, print_types);
}

namespace
{

const char* intent_to_string(variable_intent intent)
{
    switch (intent)
    {
    case variable_intent::generic:
        return "";
    case variable_intent::in_param:
        return "in ";
    case variable_intent::out_param:
        return "out ";
    case variable_intent::inout_param:
        return "inout ";
    }
}
}

void pretty_print(const function_declaration& decl, bool print_types)
{
    pretty_printer_context ctx;

    std::cout << "def entry(";

    for (const auto& param : decl.params())
    {
        std::cout << intent_to_string(param.intent());

        std::cout << ctx.get_name_for_handle(param.id());

        std::cout << ", ";
    }

    std::cout << ")";

    std::cout << " -> (" << ctx.get_name_for_handle(decl.result().id()) << ")";

    std::cout << "\n{\n";

    print(decl.body(), ctx, print_types);

    std::cout << "\n}" << std::endl;
}
}
}
