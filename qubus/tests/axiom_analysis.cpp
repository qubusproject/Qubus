#include <qubus/qubus.hpp>

#include <qubus/axiom_analysis.hpp>
#include <qubus/IR/qir.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(axiom_analysis, loop_iteration_range)
{
    using namespace qubus;

    variable_declaration i(types::integer{});

    auto loop = for_(i, integer_literal(0), integer_literal(42), sequenced_tasks({}));

    auto& body = loop->body();

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    axiom_analysis_pass analysis;

    auto result = analysis.run(*loop, analysis_man, resource_manager);

    auto axioms = result.get_valid_axioms(body);

    auto lower_bound_cond = greater_equal(variable_ref(i), integer_literal(0));
    EXPECT_EQ(axioms[0].as_expr(), *lower_bound_cond);

    auto upper_bound_cond = less(variable_ref(i), integer_literal(42));
    EXPECT_EQ(axioms[1].as_expr(), *upper_bound_cond);
}

TEST(axiom_analysis, nonunit_stride_loop_iteration_range)
{
    using namespace qubus;

    variable_declaration i(types::integer{});

    auto loop = for_(i, integer_literal(0), integer_literal(100), integer_literal(10), sequenced_tasks({}));

    auto& body = loop->body();

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    axiom_analysis_pass analysis;

    auto result = analysis.run(*loop, analysis_man, resource_manager);

    auto axioms = result.get_valid_axioms(body);

    auto lower_bound_cond = greater_equal(variable_ref(i), integer_literal(0));
    EXPECT_EQ(axioms[0].as_expr(), *lower_bound_cond);

    auto upper_bound_cond = less(variable_ref(i), integer_literal(91));
    EXPECT_EQ(axioms[1].as_expr(), *upper_bound_cond);
}

TEST(axiom_analysis, if_condition_simple)
{
    using namespace qubus;

    variable_declaration i(types::integer{});

    auto code = if_(less(variable_ref(i), integer_literal(0)), sequenced_tasks({}));

    auto& body = code->then_branch();

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    axiom_analysis_pass analysis;

    auto result = analysis.run(*code, analysis_man, resource_manager);

    auto axioms = result.get_valid_axioms(body);

    auto condition = less(variable_ref(i), integer_literal(0));
    EXPECT_EQ(axioms[0].as_expr(), *condition);
}

TEST(axiom_analysis, deep_nest)
{
    using namespace qubus;

    variable_declaration i(types::integer{});

    std::vector<std::unique_ptr<expression>> tasks;
    tasks.push_back(variable_ref(i));

    auto code = if_(less(variable_ref(i), integer_literal(0)), sequenced_tasks(std::move(tasks)));

    auto& body = code->then_branch().child(0);

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    axiom_analysis_pass analysis;

    auto result = analysis.run(*code, analysis_man, resource_manager);

    auto axioms = result.get_valid_axioms(body);

    auto condition = less(variable_ref(i), integer_literal(0));
    EXPECT_EQ(axioms[0].as_expr(), *condition);
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

    return hpx::init(argc, argv);
}


