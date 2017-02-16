#include <qubus/qubus.hpp>

#include <qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <qubus/util/unused.hpp>

#include <complex>
#include <random>
#include <vector>

#include <gtest/gtest.h>

TEST(qm_patterns, commutator)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::vector<std::complex<double>> A2(N * N);
    std::vector<std::complex<double>> B2(N * N);
    std::vector<std::complex<double>> C2(N * N);

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            A2[i * N + j] = std::complex<double>(dist(gen), dist(gen));
            B2[i * N + j] = std::complex<double>(dist(gen), dist(gen));
        }
    }

    tensor<std::complex<double>, 2> A(N, N);
    tensor<std::complex<double>, 2> B(N, N);
    tensor<std::complex<double>, 2> C(N, N);

    {
        auto A_view = get_view<host_tensor_view<std::complex<double>, 2>>(A).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                A_view(i, j) = A2[i * N + j];
            }
        }

        auto B_view = get_view<host_tensor_view<std::complex<double>, 2>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                B_view(i, j) = B2[i * N + j];
            }
        }
    }

    tensor_expr<std::complex<double>, 2> Cdef = [A, B](qtl::index i, qtl::index j) {
        qtl::index k;
        return sum(k, A(i, k) * B(k, j) - B(i, k) * A(k, j));
    };

    C = Cdef;

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            for (long int k = 0; k < N; ++k)
            {
                C2[i * N + j] += A2[i * N + k] * B2[k * N + j] - B2[i * N + k] * A2[k * N + j];
            }
        }
    }

    double error = 0.0;

    {
        auto C_view = get_view<host_tensor_view<const std::complex<double>, 2>>(C).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                std::complex<double> diff = C_view(i, j) - C2[i * N + j];

                error += std::norm(diff);
            }
        }
    }

    C.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-12);
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