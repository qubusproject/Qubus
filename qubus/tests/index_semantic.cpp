#include <qbb/qubus/qubus.hpp>

#include <qbb/qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <qbb/util/unused.hpp>

#include <vector>
#include <random>
#include <complex>

#include <gtest/gtest.h>

TEST(index_semantic, basic_index_semantic)
{
using namespace qbb::qubus;

qtl::index i, j;
qtl::index k = i; // k is an alias of i.

// Test this.
EXPECT_EQ(i, k);

// i and j are distinct indices.
EXPECT_NE(i, j);

// As are k and j.
EXPECT_NE(k, j);

EXPECT_EQ(i, i);

k = j; // rebind k

EXPECT_EQ(k, j);

EXPECT_NE(i, j);

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

