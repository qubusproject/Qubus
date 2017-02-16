#include <qbb/qubus/qubus.hpp>

#include <qbb/qubus/performance_models/symbolic_regression.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(symbolic_regression, polynomial)
{
    using namespace qbb::qubus;
    using namespace std::literals::chrono_literals;

    symbolic_regression reg(1000);

    std::vector<double> arguments(1);

    for (long int k = 0; k < 2; ++k)
    {
        for (long int i = 0; i < 100; ++i)
        {
            arguments[0] = 10.0 * i;
            auto exec_time = std::chrono::microseconds(static_cast<long int>(i*i + 10.0 * i + 100));

            reg.add_datapoint(arguments, exec_time);
        }
    }

    while (!reg.accuracy() || *reg.accuracy() > 75us)
    {
        reg.update();
    }

    std::chrono::microseconds norm(0);

    for (long int k = 0; k < 2; ++k)
    {
        for (long int i = 0; i < 100; ++i)
        {
            arguments[0] = 10.0 * i;
            auto exec_time = std::chrono::microseconds(static_cast<long int>(i*i + 10.0 * i + 100));

            auto result = reg.query(arguments);

            ASSERT_TRUE(static_cast<bool>(result));

            norm += std::chrono::microseconds(std::abs(result->count() - exec_time.count()));
        }
    }

    norm /= reg.size_of_dataset();

    EXPECT_LT(norm, 100us);
}

TEST(symbolic_regression, exponential)
{
    using namespace qbb::qubus;
    using namespace std::literals::chrono_literals;

    symbolic_regression reg(1000);

    std::vector<double> arguments(1);

    for (long int k = 0; k < 2; ++k)
    {
        for (long int i = 0; i < 100; ++i)
        {
            arguments[0] = 10.0 * i;
            auto exec_time = std::chrono::microseconds(static_cast<long int>(std::exp(0.1 * i)));

            reg.add_datapoint(arguments, exec_time);
        }
    }

    while (!reg.accuracy() || *reg.accuracy() > 75us)
    {
        reg.update();
    }

    std::chrono::microseconds norm(0);

    for (long int k = 0; k < 2; ++k)
    {
        for (long int i = 0; i < 100; ++i)
        {
            arguments[0] = 10.0 * i;
            auto exec_time = std::chrono::microseconds(static_cast<long int>(std::exp(0.1 * i)));

            auto result = reg.query(arguments);

            ASSERT_TRUE(static_cast<bool>(result));

            norm += std::chrono::microseconds(std::abs(result->count() - exec_time.count()));
        }
    }

    norm /= reg.size_of_dataset();

    EXPECT_LT(norm, 100us);
}

TEST(symbolic_regression, exponential_sliding)
{
    using namespace qbb::qubus;
    using namespace std::literals::chrono_literals;

    symbolic_regression reg(1000);

    std::vector<double> arguments(1);

    for (long int k = 0; k < 2; ++k)
    {
        for (long int i = 0; i < 100; ++i)
        {
            arguments[0] = 10.0 * i;
            auto exec_time = std::chrono::microseconds(static_cast<long int>(std::exp(0.1 * i)));

            reg.add_datapoint(arguments, exec_time);

            if (i % 10 == 0)
            {
                reg.update();
            }
        }
    }

    ASSERT_TRUE(static_cast<bool>(reg.accuracy()));

    while (*reg.accuracy() > 75us)
    {
        reg.update();
    }

    std::chrono::microseconds norm(0);

    for (long int k = 0; k < 2; ++k)
    {
        for (long int i = 0; i < 100; ++i)
        {
            arguments[0] = 10.0 * i;
            auto exec_time = std::chrono::microseconds(static_cast<long int>(std::exp(0.1 * i)));

            auto result = reg.query(arguments);

            ASSERT_TRUE(static_cast<bool>(result));

            norm += std::chrono::microseconds(std::abs(result->count() - exec_time.count()));
        }
    }

    norm /= reg.size_of_dataset();

    EXPECT_LT(norm, 100us);
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

