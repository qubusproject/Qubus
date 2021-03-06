#include <qubus/qubus.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/alias_analysis.hpp>
#include <qubus/variable_access_analysis.hpp>

#include <qubus/IR/pretty_printer.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(alias_analysis, basic_alias_analysis)
{
    using namespace qubus;

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    basic_alias_analysis_pass analysis;

    auto dummy = integer_literal(0);

    auto result = analysis.run(*dummy, analysis_man, resource_manager);

    auto A = variable_ref(variable_declaration("A", types::integer{}));
    auto B = variable_ref(variable_declaration("B", types::integer{}));

    EXPECT_EQ(result.alias(qubus::access(*A), qubus::access(*A), false), alias_result::must_alias);

    EXPECT_EQ(result.alias(qubus::access(*B), qubus::access(*B), false), alias_result::must_alias);

    EXPECT_EQ(result.alias(qubus::access(*A), qubus::access(*B), false), alias_result::noalias);

    auto A_memb1 = member_access(clone(*A), "memb1");
    auto A_memb2 = member_access(clone(*A), "memb2");
    auto B_memb = member_access(clone(*B), "memb");

    EXPECT_EQ(result.alias(qubus::access(*A_memb1), qubus::access(*A_memb2), false),
              alias_result::noalias);

    EXPECT_EQ(result.alias(qubus::access(*B_memb), qubus::access(*B_memb), false),
              alias_result::must_alias);

    EXPECT_EQ(result.alias(qubus::access(*A_memb1), qubus::access(*B_memb), false),
              alias_result::noalias);
}

TEST(alias_analysis, array_alias_analysis)
{
    using namespace qubus;

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    alias_analysis_pass analysis;
    variable_access_analysis access_analysis;

    variable_declaration i("i", types::integer{});
    variable_declaration N("N", types::integer{});

    variable_declaration A("A", types::array(types::double_{}, 1));

    auto task1 = assign(subscription(variable_ref(A), variable_ref(i)), double_literal(0.0));
    auto task2 = assign(subscription(variable_ref(A), variable_ref(i) + integer_literal(1)),
                        double_literal(1.0));

    auto body = sequenced_tasks(std::move(task1), std::move(task2));

    auto root = for_(i, integer_literal(0), variable_ref(N), integer_literal(2), std::move(body));

    auto result = analysis.run(*root, analysis_man, resource_manager);

    auto access_result = access_analysis.run(*root, analysis_man, resource_manager);

    const auto& access_set = access_result.query_accesses_for_location(root->body());

    auto accesses = access_set.get_write_accesses();

    ASSERT_EQ(accesses.size(), 2);

    EXPECT_EQ(result.alias(accesses[0], accesses[1]), alias_result::noalias);
}

TEST(alias_analysis, slice_alias_analysis)
{
    using namespace qubus;

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    alias_analysis_pass analysis;
    variable_access_analysis access_analysis;

    variable_declaration i("i", types::integer{});
    variable_declaration N("N", types::integer{});
    variable_declaration M("M", types::integer{});

    variable_declaration A("A", types::array(types::double_{}, 1));

    std::unique_ptr<expression> offset1 = var(M);
    std::unique_ptr<expression> bound1 = var(M) + integer_literal(10);

    std::unique_ptr<expression> offset2 = var(M) + integer_literal(10);
    std::unique_ptr<expression> bound2 = var(M) + integer_literal(17);

    auto task1 = assign(
        subscription(subscription(variable_ref(A), range(std::move(offset1), std::move(bound1))),
                     variable_ref(i)),
        double_literal(0.0));
    auto task2 = assign(
        subscription(subscription(variable_ref(A), range(std::move(offset2), std::move(bound2))),
                     variable_ref(i)),
        double_literal(1.0));

    auto body = sequenced_tasks(std::move(task1), std::move(task2));

    auto root = for_(i, integer_literal(0), variable_ref(N), integer_literal(2), std::move(body));

    auto result = analysis.run(*root, analysis_man, resource_manager);

    auto access_result = access_analysis.run(*root, analysis_man, resource_manager);

    const auto& access_set = access_result.query_accesses_for_location(root->body());

    auto accesses = access_set.get_write_accesses();

    ASSERT_EQ(accesses.size(), 2);

    EXPECT_EQ(result.alias(accesses[0], accesses[1]), alias_result::noalias);
}

TEST(alias_analysis, slice_with_overlap_alias_analysis)
{
    using namespace qubus;

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    alias_analysis_pass analysis;
    variable_access_analysis access_analysis;

    variable_declaration i("i", types::integer{});
    variable_declaration N("N", types::integer{});
    variable_declaration M("M", types::integer{});
    variable_declaration P("P", types::integer{});

    variable_declaration A("A", types::array(types::double_{}, 1));

    std::unique_ptr<expression> offset1 = var(M);
    std::unique_ptr<expression> bound1 = var(M) + integer_literal(10);

    std::unique_ptr<expression> offset2 = var(P) + integer_literal(10);
    std::unique_ptr<expression> bound2 = var(P) + integer_literal(17);

    auto task1 = assign(
        subscription(subscription(variable_ref(A), range(std::move(offset1), std::move(bound1))),
                     variable_ref(i)),
        double_literal(0.0));
    auto task2 = assign(
        subscription(subscription(variable_ref(A), range(std::move(offset2), std::move(bound2))),
                     variable_ref(i)),
        double_literal(1.0));

    auto body = sequenced_tasks(std::move(task1), std::move(task2));

    auto root = for_(i, integer_literal(0), variable_ref(N), integer_literal(2), std::move(body));

    auto result = analysis.run(*root, analysis_man, resource_manager);

    auto access_result = access_analysis.run(*root, analysis_man, resource_manager);

    const auto& access_set = access_result.query_accesses_for_location(root->body());

    auto accesses = access_set.get_write_accesses();

    ASSERT_EQ(accesses.size(), 2);

    EXPECT_EQ(result.alias(accesses[0], accesses[1]), alias_result::may_alias);
}

TEST(alias_analysis, 2d_slice_alias_analysis)
{
    using namespace qubus;

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    alias_analysis_pass analysis;
    variable_access_analysis access_analysis;

    variable_declaration i("i", types::integer{});
    variable_declaration N("N", types::integer{});
    variable_declaration M("M", types::integer{});

    variable_declaration A("A", types::array(types::double_{}, 2));

    std::vector<std::unique_ptr<expression>> indices, ranges1, ranges2;

    indices.push_back(var(i));

    ranges1.push_back(range(var(M), var(M) + integer_literal(10)));
    ranges1.push_back(range(integer_literal(0), integer_literal(42)));

    ranges2.push_back(
        range(var(M) + integer_literal(10), var(M) + integer_literal(17), integer_literal(1)));
    ranges2.push_back(range(integer_literal(7), integer_literal(16), integer_literal(2)));

    auto task1 =
        assign(subscription(subscription(variable_ref(A), std::move(ranges1)), clone(indices)),
               double_literal(0.0));
    auto task2 =
        assign(subscription(subscription(variable_ref(A), std::move(ranges2)), clone(indices)),
               double_literal(1.0));

    auto body = sequenced_tasks(std::move(task1), std::move(task2));

    auto root = for_(i, integer_literal(0), variable_ref(N), integer_literal(2), std::move(body));

    auto result = analysis.run(*root, analysis_man, resource_manager);

    auto access_result = access_analysis.run(*root, analysis_man, resource_manager);

    const auto& access_set = access_result.query_accesses_for_location(root->body());

    auto accesses = access_set.get_write_accesses();

    ASSERT_EQ(accesses.size(), 2);

    EXPECT_EQ(result.alias(accesses[0], accesses[1]), alias_result::noalias);
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
