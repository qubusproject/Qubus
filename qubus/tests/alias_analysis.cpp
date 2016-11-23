#include <qbb/qubus/qubus.hpp>

#include <qbb/qubus/alias_analysis.hpp>
#include <qbb/qubus/IR/qir.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(alias_analysis, basic_alias_analysis)
{
    using namespace qbb::qubus;

    auto A = variable_ref(variable_declaration(types::integer{}));
    auto B = variable_ref(variable_declaration(types::integer{}));

    EXPECT_EQ(alias_using_basic_rules(qbb::qubus::access(*A),
                                      qbb::qubus::access(*A)),
              alias_result::must_alias);

    EXPECT_EQ(alias_using_basic_rules(qbb::qubus::access(*B),
                                      qbb::qubus::access(*B)),
              alias_result::must_alias);

    EXPECT_EQ(alias_using_basic_rules(qbb::qubus::access(*A),
                                      qbb::qubus::access(*B)),
              alias_result::noalias);

    auto A_memb1 = member_access(clone(*A), "memb1");
    auto A_memb2 = member_access(clone(*A), "memb2");
    auto B_memb = member_access(clone(*B), "memb");

    EXPECT_EQ(alias_using_basic_rules(qbb::qubus::access(*A_memb1),
                                      qbb::qubus::access(*A_memb2)),
              alias_result::noalias);

    EXPECT_EQ(alias_using_basic_rules(qbb::qubus::access(*B_memb),
                                      qbb::qubus::access(*B_memb)),
              alias_result::must_alias);

    EXPECT_EQ(alias_using_basic_rules(qbb::qubus::access(*A_memb1),
                                      qbb::qubus::access(*B_memb)),
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
