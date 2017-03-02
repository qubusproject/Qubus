#include <qubus/qubus.hpp>

#include <hpx/hpx_init.hpp>

#include <qubus/util/unused.hpp>

#include <complex>
#include <random>
#include <vector>

#include <gtest/gtest.h>

TEST(scalar_support, init)
{
    using namespace qubus;

    scalar<double> a(42);

    double error = 0.0;

    {
        auto a_view = get_view<host_scalar_view<const double>>(a).get();

        auto value = a_view.get();

        ASSERT_NEAR(value, 42.0, 1e-14);
    }
}

TEST(scalar_support, assignment)
{
    using namespace qubus;

    scalar<double> a;

    a = 42;

    double error = 0.0;

    {
        auto a_view = get_view<host_scalar_view<const double>>(a).get();

        auto value = a_view.get();

        ASSERT_NEAR(value, 42.0, 1e-14);
    }
}

TEST(scalar_support, plus_assign)
{
    using namespace qubus;

    scalar<double> a(1);

    a += 42;

    double error = 0.0;

    {
        auto a_view = get_view<host_scalar_view<const double>>(a).get();

        auto value = a_view.get();

        ASSERT_NEAR(value, 43.0, 1e-14);
    }
}

TEST(scalar_support, minus_assign)
{
    using namespace qubus;

    scalar<double> a(1);

    a -= 42;

    double error = 0.0;

    {
        auto a_view = get_view<host_scalar_view<const double>>(a).get();

        auto value = a_view.get();

        ASSERT_NEAR(value, -41.0, 1e-14);
    }
}

TEST(scalar_support, mul_assign)
{
    using namespace qubus;

    scalar<double> a(1);

    a *= 42;

    double error = 0.0;

    {
        auto a_view = get_view<host_scalar_view<const double>>(a).get();

        auto value = a_view.get();

        ASSERT_NEAR(value, 42.0, 1e-14);
    }
}

TEST(scalar_support, div_assign)
{
    using namespace qubus;

    scalar<double> a(1);

    a /= 2;

    double error = 0.0;

    {
        auto a_view = get_view<host_scalar_view<const double>>(a).get();

        auto value = a_view.get();

        ASSERT_NEAR(value, 0.5, 1e-14);
    }
}

TEST(scalar_support, increment)
{
    using namespace qubus;

    scalar<util::index_t> a(1);

    a++;
    ++a;

    {
        auto a_view = get_view<host_scalar_view<const util::index_t>>(a).get();

        auto value = a_view.get();

        ASSERT_EQ(value, 3);
    }
}

TEST(scalar_support, decrement)
{
    using namespace qubus;

    scalar<util::index_t> a(42);

    --a;
    a--;

    {
        auto a_view = get_view<host_scalar_view<const util::index_t>>(a).get();

        auto value = a_view.get();

        ASSERT_EQ(value, 40);
    }
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
