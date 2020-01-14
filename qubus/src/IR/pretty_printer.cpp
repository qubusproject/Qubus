#include <hpx/config.hpp>

#include <qubus/IR/pretty_printer.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/IR/type_inference.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/util/handle.hpp>
#include <qubus/util/multi_method.hpp>
#include <qubus/util/unique_name_generator.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/unreachable.hpp>

#include <boost/optional.hpp>

#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <vector>

namespace qubus
{

namespace
{

enum class precedence_level
{
    statement = 0,
    assignment = 1,
    logical = 2,
    comparison = 3,
    addition = 4,
    multiplication = 5,
    unary = 6,
    qualifier = 7
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
        QUBUS_ASSERT(false, "default case should never be reached");
        QUBUS_UNREACHABLE();
    }
}

precedence_level get_op_precedence_level(binary_op_tag tag)
{
    switch (tag)
    {
    case binary_op_tag::assign:
    case binary_op_tag::plus_assign:
        return precedence_level::assignment;
    case binary_op_tag::logical_and:
    case binary_op_tag::logical_or:
        return precedence_level::logical;
    case binary_op_tag::equal_to:
    case binary_op_tag::not_equal_to:
    case binary_op_tag::greater:
    case binary_op_tag::less:
    case binary_op_tag::greater_equal:
    case binary_op_tag::less_equal:
        return precedence_level::comparison;
    case binary_op_tag::plus:
    case binary_op_tag::minus:
        return precedence_level::addition;
    case binary_op_tag::multiplies:
    case binary_op_tag::divides:
    case binary_op_tag::modulus:
    case binary_op_tag::div_floor:
        return precedence_level::multiplication;
    default:
        QUBUS_ASSERT(false, "default case should never be reached");
        QUBUS_UNREACHABLE();
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
        return "!";
    default:
        QUBUS_ASSERT(false, "default case should never be reached");
        QUBUS_UNREACHABLE();
    }
}

precedence_level get_op_precedence_level(unary_op_tag tag)
{
    return precedence_level::unary;
}

carrot::block print_type(const type& t)
{
    using pattern::_;

    using carrot::text;

    return text(t.name());
}

carrot::block bracket(carrot::block b, bool enabled = true)
{
    using carrot::text;

    if (enabled)
        return text("(") << std::move(b) << text(")");

    return std::move(b);
}

carrot::block print(const expression& expr, precedence_level prec_level)
{
    using carrot::text;

    using pattern::_;

    pattern::variable<std::reference_wrapper<const expression>> a, b, c, d;
    pattern::variable<util::optional_ref<const expression>> opt_expr;
    pattern::variable<binary_op_tag> btag;
    pattern::variable<unary_op_tag> utag;

    pattern::variable<std::vector<std::reference_wrapper<const expression>>> indices;
    pattern::variable<std::vector<std::reference_wrapper<const expression>>> subexprs;
    pattern::variable<std::vector<std::reference_wrapper<const expression>>> args;
    pattern::variable<std::vector<std::reference_wrapper<const expression>>> offset;
    pattern::variable<std::vector<std::reference_wrapper<const expression>>> bounds;
    pattern::variable<std::vector<std::reference_wrapper<const expression>>> strides;
    pattern::variable<std::string> id;

    pattern::variable<double> dval;
    pattern::variable<float> fval;
    pattern::variable<util::index_t> ival;
    pattern::variable<bool> bval;

    pattern::variable<std::shared_ptr<const variable_declaration>> decl;
    pattern::variable<const function&> plan;
    pattern::variable<type> t;

    pattern::variable<execution_order> order;

    pattern::variable<std::shared_ptr<const variable_declaration>> params;
    pattern::variable<std::shared_ptr<const variable_declaration>> index_decls;

    auto m =
        pattern::make_matcher<expression, carrot::block>()
            .case_(binary_operator(btag, a, b),
                   [&] {
                       auto new_prec_level = get_op_precedence_level(btag.get());

                       return bracket(print(a.get(), new_prec_level)
                                          << text(" ") << text(translate_binary_op_tag(btag.get()))
                                          << text(" ") << print(b.get(), new_prec_level),
                                      new_prec_level < prec_level);
                   })
            .case_(unary_operator(utag, a),
                   [&] {
                       auto new_prec_level = get_op_precedence_level(utag.get());

                       return bracket(text(translate_unary_op_tag(utag.get()))
                                          << print(a.get(), new_prec_level),
                                      new_prec_level < prec_level);
                   })
            .case_(variable_ref(decl), [&] { return text(decl.get()->name()); })
            // TODO: Improve float literal printing.
            .case_(double_literal(dval), [&] { return text(std::to_string(dval.get())); })
            .case_(float_literal(fval), [&] { return text(std::to_string(fval.get())); })
            .case_(integer_literal(ival), [&] { return text(std::to_string(ival.get())); })
            .case_(bool_literal(bval),
                   [&] {
                       if (bval.get())
                       {
                           return text("true");
                       }
                       else
                       {
                           return text("false");
                       }
                   })
            .case_(integer_range(a, b, c),
                   [&] {
                       carrot::block result = print(a.get(), precedence_level::statement)
                                              << text(":");

                       result = result << print(c.get(), precedence_level::statement) << text(":");

                       result = result << print(b.get(), precedence_level::statement);

                       return result;
                   })
            .case_(sequenced_tasks(subexprs),
                   [&] {
                       auto sequence = carrot::make_line(carrot::growth_direction::down);

                       for (const auto& sub_expr : subexprs.get())
                       {
                           sequence.add(print(sub_expr, precedence_level::statement));
                       }

                       return sequence;
                   })
            .case_(subscription(a, indices),
                   [&] {
                       carrot::block result = print(a.get(), precedence_level::qualifier)
                                              << text("[");

                       for (auto iter = indices.get().begin(), end = indices.get().end();
                            iter != end; ++iter)
                       {
                           result = result << print(*iter, precedence_level::statement);

                           if (iter != end - 1)
                           {
                               result = result << text(", ");
                           }
                       }

                       result = result << text("]");

                       return bracket(std::move(result), precedence_level::qualifier < prec_level);
                   })
            .case_(type_conversion(t, a),
                   [&] {
                       return text("convert")
                              << text("{") << print_type(t.get()) << text("}") << text("(")
                              << print(a.get(), precedence_level::statement) << text(")");
                   })
            .case_(unordered_tasks(subexprs),
                   [&] {
                       auto result = carrot::make_line(carrot::growth_direction::down);

                       result.add(text("unordered do"));

                       auto body = carrot::make_line(carrot::growth_direction::down);

                       for (const auto& sub_expr : subexprs.get())
                       {
                           body.add(print(sub_expr, precedence_level::statement));
                       }

                       result.add(indent(std::move(body)));
                       result.add(text("end"));

                       return result;
                   })
            .case_(intrinsic_function(id, args),
                   [&] {
                       carrot::block result = carrot::text(id.get()) << text("(");

                       for (auto iter = args.get().begin(), end = args.get().end(); iter != end;
                            ++iter)
                       {
                           result = result << print(*iter, precedence_level::statement);

                           if (iter != end - 1)
                               result = result << text(", ");
                       }

                       result = result << text(")");

                       return result;
                   })
            .case_(for_(order, decl, a, b, c, d),
                   [&] {
                       auto result = carrot::make_line(carrot::growth_direction::down);

                       carrot::block loop_head = [&order] {
                           switch (order.get())
                           {
                           case execution_order::sequential:
                               return text("for ");
                           case execution_order::unordered:
                               return text("unordered for ");
                           case execution_order::parallel:
                               return text("parallel for ");
                           }
                       }();

                       loop_head = loop_head
                                   << text(decl.get()->name()) << text(" :: ")
                                   << print_type(decl.get()->var_type()) << text(" in ")
                                   << print(a.get(), precedence_level::statement) << text(":")
                                   << print(c.get(), precedence_level::statement) << text(":")
                                   << print(b.get(), precedence_level::statement);
                       result.add(std::move(loop_head));

                       result.add(indent(print(d.get(), precedence_level::statement)));

                       result.add(text("end"));

                       return result;
                   })
            .case_(if_(a, b, opt_expr),
                   [&] {
                       auto result = carrot::make_line(carrot::growth_direction::down);

                       auto if_head = text("if ") << print(a.get(), precedence_level::statement);

                       result.add(std::move(if_head));

                       result.add(indent(print(b.get(), precedence_level::statement)));

                       if (opt_expr.get())
                       {
                           result.add(text("else"));

                           result.add(indent(print(*opt_expr.get(), precedence_level::statement)));
                       }

                       result.add(text("end"));

                       return result;
                   })
            .case_(local_variable_def(decl, a),
                   [&] {
                       return text("let ") << text(decl.get()->name()) << text(" :: ")
                                           << print_type(decl.get()->var_type()) << text(" = ")
                                           << print(a.get(), precedence_level::statement);
                   })
            /*.case_(construct(t, args),
                   [&] {
                       print_type(t.get());
                       std::cout << "(";

                       for (const auto& arg : args.get())
                       {
                           print(arg, ctx, print_types);
                           std::cout << ", ";
                       }

                       std::cout << ")";
                   })*/
            .case_(member_access(a, id), [&] {
                return bracket(print(a.get(), precedence_level::qualifier)
                                   << text(".") << text(id.get()),
                               precedence_level::qualifier < prec_level);
            });

    try
    {
        return pattern::match(expr, m);
    }
    catch (const std::logic_error&)
    {
        throw printing_error("Encountered an unknown expression.");
    }
}

carrot::block pretty_print_user_defined_type(const object_type& t)
{
    using carrot::text;

    auto result = carrot::make_line(carrot::growth_direction::down);

    auto struct_header = text("struct ") << text(t.name());

    result.add(std::move(struct_header));

    auto struct_body = carrot::make_line(carrot::growth_direction::down);

    for (const auto& member : t.properties())
    {
        auto member_declaration = text(member.id())
                                  << text(" :: ") << print_type(member.datatype());

        struct_body.add(std::move(member_declaration));
    }

    result.add(indent(std::move(struct_body)));

    result.add(text("end"));

    return result;
}
} // namespace

printing_error::printing_error(const char* what_) : std::logic_error(what_)
{
}

printing_error::printing_error(const std::string& what_) : std::logic_error(what_)
{
}

carrot::block pretty_print(const expression& expr)
{
    return print(expr, precedence_level::statement);
}

carrot::block pretty_print(const function& func)
{
    using carrot::text;

    auto result = carrot::make_line(carrot::growth_direction::down);

    auto function_declaration = text("function ") << text(func.name()) << text("(");

    for (auto iter = func.params().begin(), end = func.params().end(); iter != end; ++iter)
    {
        function_declaration = function_declaration << text((*iter)->name()) << text(" :: ")
                                                    << print_type((*iter)->var_type());

        if (iter != end - 1)
            function_declaration = function_declaration << text(", ");
    }

    function_declaration = function_declaration << text(") -> ") << text(func.result()->name())
                                                << text(" :: ")
                                                << print_type(func.result()->var_type());

    result.add(std::move(function_declaration));

    result.add(indent(pretty_print(func.body())));

    result.add(text("end"));

    return result;
}

namespace
{

struct module_type_printer
{
    carrot::block operator()(const object_type& type) const
    {
        return pretty_print_user_defined_type(type);
    }

    carrot::block operator()(const symbolic_type& type) const
    {
        throw 0; // A symbolic type can never be a part of a module.
    }

    carrot::block operator()(const void_type& type) const
    {
        throw 0; // void can never be a part of a module.
    }
};

}

carrot::block pretty_print(const module& mod)
{
    auto module_declaration = carrot::text("module ") << carrot::text(mod.id().string());

    auto module = carrot::make_line(carrot::growth_direction::down);

    module.add(std::move(module_declaration));

    module.add(carrot::text(""));

    for (const auto& type : mod.types())
    {
        module.add(visit(type, module_type_printer{}));
    }

    for (const auto& function : mod.functions())
    {
        module.add(pretty_print(function));
    }

    return module;
}
} // namespace qubus
