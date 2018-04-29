#include <qubus/IR/symbol_id.hpp>

#include <gtest/gtest.h>

TEST(symbol_id, default_constructed)
{
    qubus::symbol_id s;

    EXPECT_EQ(s.get_prefix(), qubus::symbol_id());
    EXPECT_EQ(s.string(), "");
}

TEST(symbol_id, empty_construction)
{
    EXPECT_THROW(qubus::symbol_id s("");, qubus::symbol_id_parsing_error);
}

TEST(symbol_id, simple_symbol_construction)
{
    qubus::symbol_id s("foo");

    EXPECT_EQ(s.get_prefix(), qubus::symbol_id());
    EXPECT_EQ(s.suffix(), "foo");
    EXPECT_EQ(s.string(), "foo");
}

TEST(symbol_id, nested_symbol_construction)
{
    qubus::symbol_id s("bar.foo");

    EXPECT_EQ(s.get_prefix(), qubus::symbol_id("bar"));
    EXPECT_EQ(s.suffix(), "foo");
    EXPECT_EQ(s.string(), "bar.foo");
}

TEST(symbol_id, parsing_error)
{
    EXPECT_THROW(qubus::symbol_id s("bar:foo");, qubus::symbol_id_parsing_error);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

