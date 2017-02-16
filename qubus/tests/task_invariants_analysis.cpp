#include <qubus/qubus.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/task_invariants_analysis.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(task_invariants_analysis, loop_bound_and_ind_var)
{
    using namespace qubus;

    variable_declaration var(types::array(types::double_{}, 1));

    variable_declaration i(types::integer{});
    variable_declaration j(types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(variable_ref(i));

    auto access = subscription(variable_ref(var), std::move(indices));

    auto upper_bound = subscription(member_access(variable_ref(var), "shape"), integer_literal(0));

    auto root = for_(i, integer_literal(0), clone(*upper_bound),
                     for_(j, integer_literal(0), clone(*upper_bound), std::move(access)));

    const auto& bound = root->body().child(1);
    const auto& induction_var = root->body().child(3).child(1);
    const auto& second_loop = root->body();

    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    task_invariants_analysis_pass analysis;

    auto result = analysis.run(*root, analysis_man, resource_man);

    EXPECT_TRUE(result.is_invariant(bound, *root));
    EXPECT_FALSE(result.is_invariant(induction_var, *root));
    EXPECT_TRUE(result.is_invariant(induction_var, second_loop));
}

int hpx_main(int argc, char** argv)
{
    qubus::init(argc, argv);

    auto result = RUN_ALL_TESTS();

    hpx::finalize();

    return result;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return hpx::init(argc, argv);
}
