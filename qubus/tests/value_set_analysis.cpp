#include <qubus/qubus.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/value_set_analysis.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(value_set_anaylsis, simple_array_access_in_loop_nest)
{
    using namespace qubus;

    variable_declaration var("var", types::array(types::double_{}, 1));

    variable_declaration i("i", types::integer{});
    variable_declaration N("N", types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(variable_ref(i));

    auto access = subscription(variable_ref(var), std::move(indices));

    auto root = for_(i, integer_literal(0), variable_ref(N), std::move(access));

    const auto& expr = root->body().child(1);

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    value_set_analysis_pass analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    auto value_set = result.determine_value_set(expr, *root);

    auto expected_value_set =
        isl::set(resource_man.get_isl_ctx(), "[p0] -> { [i0] : 0 <= i0 < p0 }");
    EXPECT_TRUE(is_subset(expected_value_set, value_set.values()));
}

TEST(value_set_anaylsis, array_access_in_loop_nest)
{
    using namespace qubus;

    variable_declaration var("var", types::array(types::double_{}, 1));

    variable_declaration i("i", types::integer{});
    variable_declaration j("j", types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(variable_ref(i) + variable_ref(j));

    auto access = subscription(variable_ref(var), std::move(indices));

    auto root = for_(i, integer_literal(0), integer_literal(10),
                     for_(j, integer_literal(7), integer_literal(42), std::move(access)));

    const auto& expr = root->body().child(3).child(1);

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    value_set_analysis_pass analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    auto value_set = result.determine_value_set(expr, *root);

    auto expected_value_set = isl::set(resource_man.get_isl_ctx(), "{ [i0] : 7 <= i0 <= 50 }");
    EXPECT_EQ(expected_value_set, value_set.values());
}

TEST(value_set_anaylsis, dependent_loops)
{
    using namespace qubus;

    variable_declaration var("var", types::array(types::double_{}, 1));

    variable_declaration i("i", types::integer{});
    variable_declaration j("j", types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(variable_ref(j));

    auto access = subscription(variable_ref(var), std::move(indices));

    auto root = for_(i, integer_literal(0), integer_literal(10),
                     for_(j, integer_literal(0), variable_ref(i), std::move(access)));

    const auto& expr = root->body().child(3).child(1);

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    value_set_analysis_pass analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    auto value_set = result.determine_value_set(expr, *root);

    auto expected_value_set = isl::set(resource_man.get_isl_ctx(), "{ [i0] : 0 <= i0 <= 8 }");
    EXPECT_EQ(expected_value_set, value_set.values());
}

TEST(value_set_anaylsis, blocking)
{
    using namespace qubus;

    variable_declaration var("var", types::array(types::double_{}, 1));

    variable_declaration i("i", types::integer{});
    variable_declaration j("j", types::integer{});

    variable_declaration ii("ii", types::integer{});
    variable_declaration jj("jj", types::integer{});

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

    value_set_analysis_pass analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    auto value_set = result.determine_value_set(expr, context);

    auto expected_value_set =
        isl::set(resource_man.get_isl_ctx(),
                 "[p0, p1] -> { [i0] : exists (e0 = floor((p1)/10), e1 = floor((p0)/10): 10e0 = p1 "
                 "and 10e1 = p0 and 0 <= p0 <= 990 and 0 <= p1 <= 990 and p0 <= i0 <= p0 + 9) }");

    EXPECT_EQ(expected_value_set, value_set.values());
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

    hpx::resource::partitioner rp(argc, argv, qubus::get_hpx_config(),
                                  hpx::resource::partitioner_mode::mode_allow_oversubscription);

    qubus::setup(rp);

    return hpx::init();
}
