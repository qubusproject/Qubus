#include <qubus/qubus.hpp>

#include <qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <qubus/util/unused.hpp>

#include <random>
#include <vector>

#include <gtest/gtest.h>

TEST(multi_indices, simple_expr)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    tensor<double, 2> A(N, N);

    kernel simple_expr = [A] {
        multi_index<2> ij;

        A(ij) = 42;
    };

    simple_expr();

    double error = 0.0;

    {
        auto A_view = get_view(A, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                double diff = A_view(i, j) - 42.0;

                error += diff * diff;
            }
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(multi_indices, index_splitting)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    std::vector<double> A2(N * N);

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            A2[i * N + j] = dist(gen);
        }
    }

    tensor<double, 2> A(N, N);

    {
        auto A_view = get_view(A, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                A_view(i, j) = A2[i * N + j];
            }
        }
    }

    tensor<double, 2> R(N, N);

    kernel index_splitting = [A, R] {
        multi_index<2> ij;

        qtl::index i = ij[0];
        qtl::index j = ij[1];

        R(ij) = A(i, j);
    };

    index_splitting();

    double error = 0.0;

    {
        auto R_view = get_view(R, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                double diff = R_view(i, j) - A2[i * N + j];

                error += diff * diff;
            }
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

// TODO: Should this work?
/*TEST(multi_indices, reverse_index_splitting)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    std::vector<double> A2(N * N);

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            A2[i * N + j] = dist(gen);
        }
    }

    tensor<double, 2> A(N, N);

    {
        auto A_view = get_view<host_tensor_view<double, 2>>(A).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                A_view(i, j) = A2[i * N + j];
            }
        }
    }

    tensor<double, 2> R(N, N);

    kernel reverse_index_splitting = [A, R] {
        qtl::index i, j;
        qtl::multi_index<2> ij({i, j});

        R(i, j) = A(ij);
    };

    reverse_index_splitting();

    double error = 0.0;

    {
        auto R_view = get_view<host_tensor_view<const double, 2>>(R).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                double diff = R_view(i, j) - A2[i * N + j];

                error += diff * diff;
            }
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}*/

TEST(multi_indices, multi_sum)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    std::vector<double> A2(N * N * N);
    std::vector<double> R2(N);

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            for (long int k = 0; k < N; ++k)
            {
                A2[i * N * N + j * N + k] = dist(gen);
            }
        }
    }

    tensor<double, 3> A(N, N, N);

    {
        auto A_view = get_view(A, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                for (long int k = 0; k < N; ++k)
                {
                    A_view(i, j, k) = A2[i * N * N + j * N + k];
                }
            }
        }
    }

    tensor<double, 1> R(N);

    kernel multi_sum = [A, R] {
        multi_index<2> ij;
        qtl::index k;

        R(k) = sum(ij, A(ij, k));
    };

    multi_sum();

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            for (long int k = 0; k < N; ++k)
            {
                R2[k] += A2[i * N * N + j * N + k];
            }
        }
    }

    double error = 0.0;

    {
        auto R_view = get_view(R, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            double diff = R_view(i) - R2[i];

            error += diff * diff;
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(multi_indices, multi_sum_index_splitting)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    std::vector<double> A2(N * N * N);
    std::vector<double> R2(N);

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            for (long int k = 0; k < N; ++k)
            {
                A2[i * N * N + j * N + k] = dist(gen);
            }
        }
    }

    tensor<double, 3> A(N, N, N);

    {
        auto A_view = get_view(A, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                for (long int k = 0; k < N; ++k)
                {
                    A_view(i, j, k) = A2[i * N * N + j * N + k];
                }
            }
        }
    }

    tensor<double, 1> R(N);

    kernel multi_sum_index_splitting = [A, R] {
        qtl::index i, j, k;
        multi_index<2> ij({i, j});

        R(k) = sum(ij, A(i, j, k));
    };

    multi_sum_index_splitting();

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            for (long int k = 0; k < N; ++k)
            {
                R2[k] += A2[i * N * N + j * N + k];
            }
        }
    }

    double error = 0.0;

    {
        auto R_view = get_view(R, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            double diff = R_view(i) - R2[i];

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
