#include <qubus/qubus.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/value_range_analysis.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(value_range_anaylsis, simple_array_access_in_loop_nest)
{
    using namespace qubus;

    variable_declaration var(types::array(types::double_{}, 1));

    variable_declaration i(types::integer{});
    variable_declaration N(types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(variable_ref(i));

    auto access = subscription(variable_ref(var), std::move(indices));

    auto root = for_(i, integer_literal(0), variable_ref(N), std::move(access));

    const auto& expr = root->body().child(1);

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    value_range_analysis_pass analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    auto range = result.determine_value_range(expr, *root);

    auto expected_lower_bound = integer_literal(0);
    auto expected_upper_bound = variable_ref(N);

    ASSERT_TRUE(range);
    EXPECT_EQ(*range->lower_bound, *expected_lower_bound);
    EXPECT_EQ(*range->upper_bound, *expected_upper_bound);
}

TEST(value_range_anaylsis, array_access_in_loop_nest)
{
    using namespace qubus;

    variable_declaration var(types::array(types::double_{}, 1));

    variable_declaration i(types::integer{});
    variable_declaration j(types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(variable_ref(i) + variable_ref(j));

    auto access = subscription(variable_ref(var), std::move(indices));

    auto root = for_(i, integer_literal(0), integer_literal(10),
                     for_(j, integer_literal(7), integer_literal(42), std::move(access)));

    const auto& expr = root->body().child(3).child(1);

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    value_range_analysis_pass analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    auto range = result.determine_value_range(expr, *root);

    auto expected_lower_bound = integer_literal(7);
    auto expected_upper_bound = integer_literal(51);

    ASSERT_TRUE(range);
    EXPECT_EQ(*range->lower_bound, *expected_lower_bound);
    EXPECT_EQ(*range->upper_bound, *expected_upper_bound);
}

TEST(value_range_anaylsis, dependent_loops)
{
    using namespace qubus;

    variable_declaration var(types::array(types::double_{}, 1));

    variable_declaration i(types::integer{});
    variable_declaration j(types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(variable_ref(j));

    auto access = subscription(variable_ref(var), std::move(indices));

    auto root = for_(i, integer_literal(0), integer_literal(10),
                     for_(j, integer_literal(0), variable_ref(i), std::move(access)));

    const auto& expr = root->body().child(3).child(1);

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    value_range_analysis_pass analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    auto range = result.determine_value_range(expr, *root);

    auto expected_lower_bound = integer_literal(0);
    auto expected_upper_bound = integer_literal(9);

    ASSERT_TRUE(range);
    EXPECT_EQ(*range->lower_bound, *expected_lower_bound);
    EXPECT_EQ(*range->upper_bound, *expected_upper_bound);
}

TEST(value_range_anaylsis, blocking)
{
    using namespace qubus;

    variable_declaration var(types::array(types::double_{}, 1));

    variable_declaration i(types::integer{});
    variable_declaration j(types::integer{});

    variable_declaration ii(types::integer{});
    variable_declaration jj(types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(variable_ref(jj));

    auto access = subscription(variable_ref(var), std::move(indices));

    auto inner_block =
            for_(ii, variable_ref(i), variable_ref(i) + integer_literal(10),
                 for_(jj, variable_ref(j), variable_ref(j) + integer_literal(10), std::move(access)));

    auto root = for_(i, integer_literal(0), integer_literal(1000), integer_literal(10),
                     for_(j, integer_literal(0), integer_literal(1000), integer_literal(10),
                          std::move(inner_block)));

    const auto& expr = root->body().child(3).child(3).child(3).child(1);
    const auto& context = root->body().child(3);

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    value_range_analysis_pass analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    auto range = result.determine_value_range(expr, context);

    auto expected_lower_bound = variable_ref(j);
    auto expected_upper_bound = variable_ref(j) + integer_literal(10);

    ASSERT_TRUE(range);
    EXPECT_EQ(*range->lower_bound, *expected_lower_bound);
    EXPECT_EQ(*range->upper_bound, *expected_upper_bound);
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

