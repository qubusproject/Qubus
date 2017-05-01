#include <qubus/qubus.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/variable_access_analysis.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(variable_access_analysis, scalar_write)
{
    using namespace qubus;

    variable_declaration var(types::double_{});

    variable_declaration i(types::integer{});
    variable_declaration N(types::integer{});

    auto access = assign(variable_ref(var), lit(42));

    auto root = for_(i, integer_literal(0), variable_ref(N), std::move(access));

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    variable_access_analysis analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    const auto& access_set = result.query_accesses_for_location(*root);

    const auto& write_accesses = access_set.get_write_accesses();
    const auto& read_accesses = access_set.get_read_accesses();

    EXPECT_TRUE(read_accesses.empty());

    ASSERT_EQ(write_accesses.size(), 1);

    EXPECT_EQ(write_accesses[0].variable(), var);
    EXPECT_FALSE(write_accesses[0].is_qualified());
}

TEST(variable_access_analysis, array_write)
{
    using namespace qubus;

    variable_declaration var(types::array(types::double_{}, 1));

    variable_declaration i(types::integer{});
    variable_declaration N(types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(variable_ref(i));

    auto access = assign(subscription(variable_ref(var), std::move(indices)), lit(42));

    auto root = for_(i, integer_literal(0), variable_ref(N), std::move(access));

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    variable_access_analysis analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    const auto& access_set = result.query_accesses_for_location(*root);

    const auto& write_accesses = access_set.get_write_accesses();
    const auto& read_accesses = access_set.get_read_accesses();

    EXPECT_TRUE(read_accesses.empty());

    ASSERT_EQ(write_accesses.size(), 1);

    EXPECT_EQ(write_accesses[0].variable(), var);
    EXPECT_TRUE(write_accesses[0].is_qualified());
}

TEST(variable_access_analysis, copy)
{
    using namespace qubus;

    variable_declaration A(types::array(types::double_{}, 1));
    variable_declaration B(types::array(types::double_{}, 1));

    variable_declaration i(types::integer{});
    variable_declaration N(types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(variable_ref(i));

    auto access = assign(subscription(variable_ref(A), clone(indices)),
                         subscription(variable_ref(B), clone(indices)));

    auto root = for_(i, integer_literal(0), variable_ref(N), std::move(access));

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    variable_access_analysis analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    const auto& access_set = result.query_accesses_for_location(*root);

    const auto& write_accesses = access_set.get_write_accesses();
    const auto& read_accesses = access_set.get_read_accesses();

    ASSERT_EQ(read_accesses.size(), 1);
    ASSERT_EQ(write_accesses.size(), 1);

    EXPECT_EQ(read_accesses[0].variable(), B);
    EXPECT_TRUE(read_accesses[0].is_qualified());

    EXPECT_EQ(write_accesses[0].variable(), A);
    EXPECT_TRUE(write_accesses[0].is_qualified());
}

TEST(variable_access_analysis, indirect_addressing)
{
    using namespace qubus;

    variable_declaration var(types::array(types::double_{}, 1));
    variable_declaration indices(types::array(types::integer{}, 1));

    variable_declaration i(types::integer{});
    variable_declaration N(types::integer{});

    auto idx = subscription(variable_ref(indices), variable_ref(i));

    auto access = assign(subscription(variable_ref(var), std::move(idx)), lit(42));

    auto root = for_(i, integer_literal(0), variable_ref(N), std::move(access));

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    variable_access_analysis analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    const auto& access_set = result.query_accesses_for_location(*root);

    const auto& write_accesses = access_set.get_write_accesses();
    const auto& read_accesses = access_set.get_read_accesses();

    ASSERT_EQ(read_accesses.size(), 1);
    ASSERT_EQ(write_accesses.size(), 1);

    EXPECT_EQ(read_accesses[0].variable(), indices);
    EXPECT_TRUE(read_accesses[0].is_qualified());

    EXPECT_EQ(write_accesses[0].variable(), var);
    EXPECT_TRUE(write_accesses[0].is_qualified());
}

int hpx_main(int argc, char** argv)
{
    qubus::init(argc, argv);

    auto result = RUN_ALL_TESTS();

    qubus::finalize();

    hpx::finalize();

    return result;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return hpx::init(argc, argv, qubus::get_hpx_config());
}
