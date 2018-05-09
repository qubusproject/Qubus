#include <qubus/IR/parsing.hpp>
#include <qubus/IR/module.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

#include <string>
#include <fstream>
#include <streambuf>

std::string read_code(const std::string& filepath)
{
    std::ifstream fin(filepath);

    auto first = std::istreambuf_iterator<char>(fin);
    auto last = std::istreambuf_iterator<char>();

    std::string code(first, last);

    return code;
}

TEST(module, serialization)
{
    std::vector<char> buffer;

    {
        hpx::serialization::output_archive oar(buffer);

        auto code = read_code("samples/empty_function");

        auto mod = qubus::parse_qir(code);

        ASSERT_EQ(mod->id(), qubus::symbol_id("test"));

        for (const auto& func : mod->functions())
        {
            ASSERT_EQ(func.name(), "empty");
        }

        oar << mod;
    }

    {
        hpx::serialization::input_archive iar(buffer);

        std::unique_ptr<qubus::module> mod;

        iar >> mod;

        EXPECT_EQ(mod->id(), qubus::symbol_id("test"));

        for (const auto& func : mod->functions())
        {
            EXPECT_EQ(func.name(), "empty");
        }
    }
}

int hpx_main(int argc, char** argv)
{
    auto result = RUN_ALL_TESTS();

    hpx::finalize();

    return result;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return hpx::init(argc, argv);
}

