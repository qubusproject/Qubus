#include <qbb/qubus/qubus.hpp>

#include <hpx/hpx_init.hpp>

#include <qbb/util/unused.hpp>

#include <vector>
#include <random>
#include <complex>

#include <gtest/gtest.h>

TEST(qm_patterns, commutator)
{
    using namespace qbb::qubus;

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

    qbb::qubus::index i("i");
    qbb::qubus::index j("j");
    qbb::qubus::index k("k");

    tensor<std::complex<double>, 2> A(N, N);
    tensor<std::complex<double>, 2> B(N, N);
    tensor<std::complex<double>, 2> C(N, N);

    auto init_A = make_plan()
                      .body([&](cpu_tensor_view<std::complex<double>, 2> A)
                            {
                                for (long int i = 0; i < N; ++i)
                                {
                                    for (long int j = 0; j < N; ++j)
                                    {
                                        A(i, j) = A2[i * N + j];
                                    }
                                }
                            })
                      .finalize();

    auto init_B = make_plan()
                      .body([&](cpu_tensor_view<std::complex<double>, 2> B)
                            {
                                for (long int i = 0; i < N; ++i)
                                {
                                    for (long int j = 0; j < N; ++j)
                                    {
                                        B(i, j) = B2[i * N + j];
                                    }
                                }
                            })
                      .finalize();

    execute(init_A, A);
    execute(init_B, B);

    tensor_expr<std::complex<double>, 2> Cdef =
        def_tensor(i, j)[sum(A(i, k) * B(k, j) - B(i, k) * A(k, j), k)];

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

    double error;

    auto test = make_plan()
                    .body([&](cpu_tensor_view<std::complex<double>, 2> C)
                          {
                              error = 0.0;

                              for (long int i = 0; i < N; ++i)
                              {
                                  for (long int j = 0; j < N; ++j)
                                  {
                                      std::complex<double> diff = C(i, j) - C2[i * N + j];

                                      error += std::norm(diff);
                                  }
                              }
                          })
                    .finalize();

    execute(test, C);
    C.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-12);
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