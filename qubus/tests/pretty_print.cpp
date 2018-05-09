#include <qubus/IR/pretty_printer.hpp>
#include <qubus/IR/qir.hpp>

#include <gtest/gtest.h>

std::string render_expression(const qubus::expression& expr)
{
    auto block = qubus::pretty_print(expr);

    carrot::plain_form form;

    carrot::render(block, form);

    auto target = carrot::get_stdout_target();

    auto result = form.to_string(target);

    if (!result.empty())
    {
        result.pop_back();
    }

    return result;
}

std::string render_expression(std::unique_ptr<qubus::expression> expr)
{
    return render_expression(*expr);
}

TEST(pretty_print, function)
{
    qubus::module mod(qubus::symbol_id("test"));

    std::vector<qubus::variable_declaration> params;
    params.emplace_back("a", qubus::types::integer{});
    params.emplace_back("b", qubus::types::integer{});

    qubus::variable_declaration result("r", qubus::types::integer{});

    auto body = qubus::sequenced_tasks({});

    mod.add_function("foo", std::move(params), std::move(result), std::move(body));

    auto block = qubus::pretty_print(mod);

    carrot::plain_form form;

    carrot::render(block, form);

    auto target = carrot::get_stdout_target();

    auto code = form.to_string(target);

    EXPECT_EQ(code, "module test\n\nfunction foo(a :: Int, b :: Int) -> r :: Int\nend\n");
}

TEST(pretty_print, expression)
{
    qubus::module mod(qubus::symbol_id("test"));

    qubus::variable_declaration a("a", qubus::types::integer{});
    qubus::variable_declaration b("b", qubus::types::integer{});

    std::vector<qubus::variable_declaration> params;
    params.push_back(a);
    params.push_back(b);

    qubus::variable_declaration r("r", qubus::types::integer{});

    auto body = assign(var(r), var(a) * (var(b) + qubus::lit(1)));

    mod.add_function("foo", std::move(params), r, std::move(body));

    auto block = qubus::pretty_print(mod);

    carrot::plain_form form;

    carrot::render(block, form);

    auto target = carrot::get_stdout_target();

    auto code = form.to_string(target);

    EXPECT_EQ(
        code,
        "module test\n\nfunction foo(a :: Int, b :: Int) -> r :: Int\n    r = a * (b + 1)\nend\n");
}

TEST(pretty_print, struct_)
{
    qubus::module mod(qubus::symbol_id("test"));

    std::vector<qubus::types::struct_::member> members;

    members.emplace_back(qubus::types::double_(), "a");
    members.emplace_back(qubus::types::integer(), "b");

    qubus::types::struct_ foo("Foo", std::move(members));

    mod.add_type(std::move(foo));

    auto block = qubus::pretty_print(mod);

    carrot::plain_form form;

    carrot::render(block, form);

    auto target = carrot::get_stdout_target();

    auto code = form.to_string(target);

    EXPECT_EQ(code, "module test\n\nstruct Foo\n    a :: Double\n    b :: Int\nend\n");
}

TEST(pretty_print, integer_literal)
{
    EXPECT_EQ(render_expression(qubus::lit(42)), "42");
    EXPECT_EQ(render_expression(qubus::lit(-1)), "-1");
}

TEST(pretty_print, float_literal)
{
    EXPECT_EQ(render_expression(qubus::lit(3.141)), "3.141000");
    EXPECT_EQ(render_expression(qubus::lit(-1.0)), "-1.000000");
    EXPECT_EQ(render_expression(qubus::lit(1e13)), "10000000000000.000000");
}

TEST(pretty_print, bool_literal)
{
    EXPECT_EQ(render_expression(qubus::lit(true)), "true");
    EXPECT_EQ(render_expression(qubus::lit(false)), "false");
}

TEST(pretty_print, binary_operators)
{
    EXPECT_EQ(render_expression(qubus::lit(42) + qubus::lit(1)), "42 + 1");
    EXPECT_EQ(render_expression(qubus::lit(42) - qubus::lit(1)), "42 - 1");
    EXPECT_EQ(render_expression(qubus::lit(42) * qubus::lit(1)), "42 * 1");
    EXPECT_EQ(render_expression(qubus::lit(42) / qubus::lit(1)), "42 / 1");
    EXPECT_EQ(render_expression(qubus::lit(42) % qubus::lit(1)), "42 % 1");
    EXPECT_EQ(render_expression(div_floor(qubus::lit(42), qubus::lit(1))), "42 // 1");

    EXPECT_EQ(render_expression(equal_to(qubus::lit(42), qubus::lit(1))), "42 == 1");
    EXPECT_EQ(render_expression(not_equal_to(qubus::lit(42), qubus::lit(1))), "42 != 1");
    EXPECT_EQ(render_expression(less(qubus::lit(42), qubus::lit(1))), "42 < 1");
    EXPECT_EQ(render_expression(greater(qubus::lit(42), qubus::lit(1))), "42 > 1");
    EXPECT_EQ(render_expression(less_equal(qubus::lit(42), qubus::lit(1))), "42 <= 1");
    EXPECT_EQ(render_expression(greater_equal(qubus::lit(42), qubus::lit(1))), "42 >= 1");

    EXPECT_EQ(render_expression(logical_and(qubus::lit(true), qubus::lit(false))), "true && false");
    EXPECT_EQ(render_expression(logical_or(qubus::lit(true), qubus::lit(false))), "true || false");
}

TEST(pretty_print, unary_operators)
{
    EXPECT_EQ(render_expression(+qubus::lit(42)), "+42");
    EXPECT_EQ(render_expression(-qubus::lit(42)), "-42");

    EXPECT_EQ(render_expression(logical_not(qubus::lit(false))), "!false");
}

TEST(pretty_print, assignment)
{
    qubus::variable_declaration foo("foo", qubus::types::double_{});

    EXPECT_EQ(render_expression(assign(var(foo), qubus::lit(42))), "foo = 42");
    EXPECT_EQ(render_expression(plus_assign(var(foo), qubus::lit(42))), "foo += 42");
}

TEST(pretty_print, variable)
{
    qubus::variable_declaration bar("bar", qubus::types::double_{});

    EXPECT_EQ(render_expression(var(bar)), "bar");
}

TEST(pretty_print, member_access)
{
    qubus::types::struct_ foo("foo",
                              {qubus::types::struct_::member(qubus::types::double_{}, "bar")});

    qubus::variable_declaration basr("basr", foo);

    EXPECT_EQ(render_expression(member_access(var(basr), "bar")), "basr.bar");
}

TEST(pretty_print, subscription)
{
    qubus::variable_declaration basr("basr", qubus::types::array(qubus::types::double_{}, 1));
    qubus::variable_declaration foo("foo", qubus::types::array(qubus::types::double_{}, 2));

    EXPECT_EQ(render_expression(subscription(var(basr), qubus::lit(0))), "basr[0]");

    std::vector<std::unique_ptr<qubus::expression>> indices;

    indices.push_back(qubus::lit(0));
    indices.push_back(qubus::lit(1));

    EXPECT_EQ(render_expression(subscription(var(foo), std::move(indices))), "foo[0, 1]");
}

TEST(pretty_print, slicing)
{
    qubus::variable_declaration basr("basr", qubus::types::array(qubus::types::double_{}, 1));
    qubus::variable_declaration foo("foo", qubus::types::array(qubus::types::double_{}, 2));

    EXPECT_EQ(render_expression(
                  subscription(var(basr), range(qubus::lit(0), qubus::lit(5), qubus::lit(2)))),
              "basr[0:2:5]");

    std::vector<std::unique_ptr<qubus::expression>> indices;

    indices.push_back(range(qubus::lit(0), qubus::lit(3), qubus::lit(2)));
    indices.push_back(range(qubus::lit(1), qubus::lit(4), qubus::lit(1)));

    EXPECT_EQ(render_expression(subscription(var(foo), std::move(indices))), "foo[0:2:3, 1:1:4]");
}

TEST(pretty_print, sequenced_tasks)
{
    qubus::variable_declaration foo("foo", qubus::types::double_{});
    qubus::variable_declaration bar("bar", qubus::types::double_{});

    EXPECT_EQ(render_expression(sequenced_tasks(assign(var(foo), qubus::lit(42)),
                                                assign(var(bar), qubus::lit(4)))),
              "foo = 42\nbar = 4");
}

TEST(pretty_print, unordered_tasks)
{
    qubus::variable_declaration foo("foo", qubus::types::double_{});
    qubus::variable_declaration bar("bar", qubus::types::double_{});

    EXPECT_EQ(render_expression(unordered_tasks(assign(var(foo), qubus::lit(42)),
                                                assign(var(bar), qubus::lit(4)))),
              "unordered do\n    foo = 42\n    bar = 4\nend");
}

TEST(pretty_print, for_loop)
{
    qubus::variable_declaration foo("foo", qubus::types::double_{});
    qubus::variable_declaration i("i", qubus::types::integer{});

    EXPECT_EQ(render_expression(for_(i, qubus::lit(0), qubus::lit(10), qubus::lit(2),
                                     assign(var(foo), qubus::lit(42)))),
              "for i :: Int in 0:2:10\n    foo = 42\nend");
    EXPECT_EQ(render_expression(unordered_for(i, qubus::lit(0), qubus::lit(10), qubus::lit(2),
                                              assign(var(foo), qubus::lit(42)))),
              "unordered for i :: Int in 0:2:10\n    foo = 42\nend");
    EXPECT_EQ(render_expression(parallel_for(i, qubus::lit(0), qubus::lit(10), qubus::lit(2),
                                             assign(var(foo), qubus::lit(42)))),
              "parallel for i :: Int in 0:2:10\n    foo = 42\nend");
}

TEST(pretty_print, control_flow)
{
    qubus::variable_declaration foo("foo", qubus::types::double_{});
    qubus::variable_declaration bar("bar", qubus::types::integer{});

    EXPECT_EQ(
        render_expression(if_(equal_to(var(foo), var(bar)), assign(var(foo), qubus::lit(42)))),
        "if foo == bar\n    foo = 42\nend");
    EXPECT_EQ(render_expression(if_(equal_to(var(foo), var(bar)), assign(var(foo), qubus::lit(42)),
                                    assign(var(bar), qubus::lit(4)))),
              "if foo == bar\n    foo = 42\nelse\n    bar = 4\nend");
}

TEST(pretty_print, local_var_def)
{
    qubus::variable_declaration foo("foo", qubus::types::double_{});

    EXPECT_EQ(render_expression(local_variable_def(foo, qubus::lit(42))), "let foo :: Double = 42");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
