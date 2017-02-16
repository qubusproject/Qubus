#include <qbb/qubus/qubus.hpp>

#include <qbb/qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <qbb/util/unused.hpp>

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

    tensor_expr<double, 2> Adef = [N](qtl::index i, qtl::index j) { return delta(N, i, j); };

    A = Adef;

    double error = 0.0;

    {
        auto A_view = get_view<host_tensor_view<const double, 2>>(A).get();

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

    tensor_expr<double, 1> Adef = [N](qtl::index i) {
        qtl::index j;
        return sum(j, delta(N, i, j));
    };

    A = Adef;

    double error = 0.0;

    {
        auto A_view = get_view<host_tensor_view<const double, 1>>(A).get();

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

    hpx::finalize();

    return result;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return hpx::init(argc, argv);
}
