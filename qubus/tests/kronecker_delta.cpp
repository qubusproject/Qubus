#include <qubus/qubus.hpp>

#include <qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <qubus/util/unused.hpp>

#include <complex>
#include <random>
#include <vector>

#include <gtest/gtest.h>

TEST(kronecker_delta, identity)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 10;

    tensor<double, 2> A(N, N);

    kernel identify = [A, N] { qtl::index i, j; A(i, j) = delta(N, i, j); };

    identify();

    double error = 0.0;

    {
        auto A_view = get_view(A, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                double diff = A_view(i, j) - (i == j ? 1.0 : 0.0);

                error += diff * diff;
            }
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(kronecker_delta, identity_contraction)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    tensor<double, 1> A(N);

    kernel identify_contraction = [A, N] { qtl::index i, j; A(i) = sum(j, delta(N, i, j)); };

    identify_contraction();

    double error = 0.0;

    {
        auto A_view = get_view(A, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            double diff = A_view(i) - 1.0;

            error += diff * diff;
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
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
