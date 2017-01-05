#include <qbb/qubus/qubus.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/alias_analysis.hpp>
#include <qbb/qubus/variable_access_analysis.hpp>

#include <qbb/qubus/IR/pretty_printer.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(alias_analysis, basic_alias_analysis)
{
    using namespace qbb::qubus;

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    basic_alias_analysis_pass analysis;

    auto dummy = integer_literal(0);

    auto result = analysis.run(*dummy, analysis_man, resource_manager);

    auto A = variable_ref(variable_declaration(types::integer{}));
    auto B = variable_ref(variable_declaration(types::integer{}));

    EXPECT_EQ(result.alias(qbb::qubus::access(*A), qbb::qubus::access(*A), false),
              alias_result::must_alias);

    EXPECT_EQ(result.alias(qbb::qubus::access(*B), qbb::qubus::access(*B), false),
              alias_result::must_alias);

    EXPECT_EQ(result.alias(qbb::qubus::access(*A), qbb::qubus::access(*B), false),
              alias_result::noalias);

    auto A_memb1 = member_access(clone(*A), "memb1");
    auto A_memb2 = member_access(clone(*A), "memb2");
    auto B_memb = member_access(clone(*B), "memb");

    EXPECT_EQ(result.alias(qbb::qubus::access(*A_memb1), qbb::qubus::access(*A_memb2), false),
              alias_result::noalias);

    EXPECT_EQ(result.alias(qbb::qubus::access(*B_memb), qbb::qubus::access(*B_memb), false),
              alias_result::must_alias);

    EXPECT_EQ(result.alias(qbb::qubus::access(*A_memb1), qbb::qubus::access(*B_memb), false),
              alias_result::noalias);
}

TEST(alias_analysis, array_alias_analysis)
{
    using namespace qbb::qubus;

    pass_resource_manager resource_manager;
    analysis_manager analysis_man(resource_manager);

    alias_analysis_pass analysis;
    variable_access_analysis access_analysis;

    variable_declaration i(types::integer{});
    variable_declaration N(types::integer{});

    variable_declaration A(types::array(types::double_{}));

    auto task1 = assign(subscription(variable_ref(A), variable_ref(i)), double_literal(0.0));
    auto task2 = assign(subscription(variable_ref(A), variable_ref(i) + integer_literal(1)), double_literal(1.0));

    auto body = sequenced_tasks(std::move(task1), std::move(task2));

    auto root = for_(i, integer_literal(0), variable_ref(N), integer_literal(2), std::move(body));

    auto result = analysis.run(*root, analysis_man, resource_manager);

    auto access_result = access_analysis.run(*root, analysis_man, resource_manager);

    const auto& access_set = access_result.query_accesses_for_location(root->body());

    auto accesses = access_set.get_write_accesses();

    ASSERT_EQ(accesses.size(), 2);

    EXPECT_EQ(result.alias(accesses[0], accesses[1]),
              alias_result::noalias);
}

int hpx_main(int argc, char** argv)
{
    qbb::qubus::init(argc, argv);

    auto result = RUN_ALL_TESTS();

    hpx::finalize();

    return result;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return hpx::init(argc, argv);
}
