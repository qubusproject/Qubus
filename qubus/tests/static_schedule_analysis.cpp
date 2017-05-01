#include <qubus/qubus.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/static_schedule_analysis.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(static_schedule_analysis, empty_loop)
{
    using namespace qubus;

    variable_declaration i(types::integer{});

    auto loop = for_(i, integer_literal(0), integer_literal(42),
                     if_(less(var(i), integer_literal(21)), sequenced_tasks({})));

    auto& body = loop->body();

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    static_schedule_analysis_pass analysis;

    auto result = analysis.run(*loop, analysis_man, resource_manager);

    auto schedule = result.get_schedule_containing(body);

    ASSERT_TRUE(schedule);

    auto node = schedule->get_schedule_node(*loop);

    auto schedule_map = node.get_subtree_schedule_union_map();

    auto empty_map = isl::union_map::empty(schedule_map.get_space());

    EXPECT_EQ(schedule_map, empty_map);
}

TEST(static_schedule_analysis, zero_init)
{
    using namespace qubus;

    variable_declaration i(types::integer{});

    variable_declaration A(types::array(types::double_{}, 1));

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(var(i));

    auto loop = for_(i, integer_literal(0), integer_literal(42),
                     assign(subscription(var(A), std::move(indices)), integer_literal(0)));

    auto& body = loop->body();

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    auto& isl_ctx = resource_manager.get_isl_ctx();

    static_schedule_analysis_pass analysis;

    auto result = analysis.run(*loop, analysis_man, resource_manager);

    auto schedule = result.get_schedule_containing(body);

    ASSERT_TRUE(schedule);

    auto node = schedule->get_schedule_node(*loop);

    auto schedule_map = node.get_subtree_schedule_union_map();

    auto expected_schedule_map =
        isl::union_map(isl_ctx, "{ stmt0[i0] -> [i0] : i0 >= 0 and i0 <= 41 }");

    EXPECT_EQ(schedule_map, expected_schedule_map);
}

TEST(static_schedule_analysis, strip_mined_loop)
{
    using namespace qubus;

    variable_declaration i(types::integer{});
    variable_declaration ii(types::integer{});
    variable_declaration N(types::integer{});

    variable_declaration A(types::array(types::double_{}, 1));

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(var(ii));

    auto loop = for_(i, integer_literal(0), var(N), integer_literal(3),
                     for_(ii, var(i), var(i) + integer_literal(3),
                          assign(subscription(var(A), std::move(indices)), integer_literal(0))));

    auto& body = loop->body();

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    auto& isl_ctx = resource_manager.get_isl_ctx();

    static_schedule_analysis_pass analysis;

    auto result = analysis.run(*loop, analysis_man, resource_manager);

    auto schedule = result.get_schedule_containing(body);

    ASSERT_TRUE(schedule);

    auto node = schedule->get_schedule_node(*loop);

    auto schedule_map = node.get_subtree_schedule_union_map();

    auto expected_schedule_map = isl::union_map(
        isl_ctx, "[p0] -> { stmt0[i0, i1] -> [i0, i1] : exists (e0 = floor((i0)/3): 3e0 = i0 and "
                 "i0 >= 0 and i0 < p0 and i1 >= i0 and i1 <= 2 + i0) }");

    EXPECT_EQ(schedule_map, expected_schedule_map);
}

TEST(static_schedule_analysis, multiple_statements)
{
    using namespace qubus;

    variable_declaration i(types::integer{});
    variable_declaration N(types::integer{});

    variable_declaration A(types::array(types::double_{}, 1));

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(var(i));

    std::vector<std::unique_ptr<expression>> tasks;
    tasks.push_back(assign(subscription(var(A), std::move(indices)), integer_literal(0)));
    tasks.push_back(assign(subscription(var(A), std::move(indices)), integer_literal(42)));

    auto loop = for_(i, integer_literal(0), var(N), sequenced_tasks(std::move(tasks)));

    auto& body = loop->body();

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    auto& isl_ctx = resource_manager.get_isl_ctx();

    static_schedule_analysis_pass analysis;

    auto result = analysis.run(*loop, analysis_man, resource_manager);

    auto schedule = result.get_schedule_containing(body);

    ASSERT_TRUE(schedule);

    auto node = schedule->get_schedule_node(*loop);

    auto schedule_map = node.get_subtree_schedule_union_map();

    auto expected_schedule_map =
        isl::union_map(isl_ctx, "[p0] -> { stmt0[i0] -> [i0, 0] : i0 >= 0 and i0 < p0; "
                                "stmt1[i0] -> [i0, 1] : i0 >= 0 and i0 < p0 }");

    EXPECT_EQ(schedule_map, expected_schedule_map);
}

TEST(static_schedule_analysis, loop_with_guard)
{
    using namespace qubus;

    variable_declaration i(types::integer{});
    variable_declaration N(types::integer{});

    variable_declaration A(types::array(types::double_{}, 1));

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(var(i));

    auto loop = for_(i, integer_literal(0), var(N),
                     if_(less(var(i), var(N) / integer_literal(2)),
                         assign(subscription(var(A), std::move(indices)), integer_literal(0)),
                         assign(subscription(var(A), std::move(indices)), integer_literal(42))));

    auto& body = loop->body();

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    auto& isl_ctx = resource_manager.get_isl_ctx();

    static_schedule_analysis_pass analysis;

    auto result = analysis.run(*loop, analysis_man, resource_manager);

    auto schedule = result.get_schedule_containing(body);

    ASSERT_TRUE(schedule);

    auto node = schedule->get_schedule_node(*loop);

    auto schedule_map = node.get_subtree_schedule_union_map();

    auto expected_schedule_map =
        isl::union_map(isl_ctx, "[p0] -> { stmt0[i0] -> [i0, 0] : i0 >= 0 and 2i0 <= -2 + p0; "
                                "stmt1[i0] -> [i0, 1] : i0 < p0 and 2i0 >= -1 + p0 }");

    EXPECT_EQ(schedule_map, expected_schedule_map);
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
