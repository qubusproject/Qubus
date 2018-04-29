#include <qubus/IR/parsing.hpp>
#include <qubus/IR/module.hpp>

#include <gtest/gtest.h>

#include <string>
#include <iostream>
#include <streambuf>

std::string read_code(const std::string& filepath)
{
    std::ifstream fin(filepath);

    auto first = std::istreambuf_iterator<char>(fin);
    auto last = std::istreambuf_iterator<char>();

    std::string code(first, last);

    return code;
}

TEST(parsing, matrix_vector_multiplication)
{
    auto code = read_code("samples/matrix_vector_multiplication");

    auto mod = qubus::parse_qir(code);

    EXPECT_EQ(mod->id(), qubus::symbol_id("test"));
}

TEST(parsing, undefined_variable)
{
    auto code = read_code("samples/undefined_variable");

    EXPECT_THROW(qubus::parse_qir(code);, qubus::parsing_error);
}

TEST(parsing, missing_end)
{
    auto code = read_code("samples/missing_end");

    EXPECT_THROW(qubus::parse_qir(code);, qubus::parsing_error);
}

TEST(parsing, missing_type)
{
    auto code = read_code("samples/missing_type");

    EXPECT_THROW(qubus::parse_qir(code);, qubus::parsing_error);
}

TEST(parsing, missing_module_id)
{
    auto code = read_code("samples/missing_module_id");

    EXPECT_THROW(qubus::parse_qir(code);, qubus::parsing_error);
}

TEST(parsing, dot_product)
{
    auto code = read_code("samples/dot_product");

    qubus::parse_qir(code);
}

TEST(parsing, simple_expression)
{
    auto code = read_code("samples/simple_expression");

    qubus::parse_qir(code);
}

TEST(parsing, wrong_type_param)
{
    auto code = read_code("samples/wrong_type_param");

    EXPECT_THROW(qubus::parse_qir(code);, qubus::parsing_error);
}

TEST(parsing, is_negative)
{
    auto code = read_code("samples/is_negative");

    qubus::parse_qir(code);
}

TEST(parsing, empty_function)
{
    auto code = read_code("samples/empty_function");

    qubus::parse_qir(code);
}

TEST(parsing, slow_unit_1)
{
    auto code = read_code("samples/slow-unit-1");

    EXPECT_THROW(qubus::parse_qir(code);, qubus::parsing_error);
}

TEST(parsing, slow_unit_2)
{
    auto code = read_code("samples/slow-unit-2");

    EXPECT_THROW(qubus::parse_qir(code);, qubus::parsing_error);
}

TEST(parsing, symbol_out_of_scope)
{
    auto code = read_code("samples/symbol_out_of_scope");

    EXPECT_THROW(qubus::parse_qir(code);, qubus::parsing_error);
}

TEST(parsing, slicing)
{
    auto code = read_code("samples/slicing");

    qubus::parse_qir(code);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

