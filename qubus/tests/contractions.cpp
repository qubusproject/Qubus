#include <qubus/qubus.hpp>

#include <qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <qubus/util/unused.hpp>

#include <complex>
#include <random>
#include <vector>

#include <gtest/gtest.h>

TEST(contractions, simple_contraction)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::vector<double> A2(N * N);
    std::vector<double> B2(N * N);
    std::vector<double> C2(N * N);

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            A2[i * N + j] = dist(gen);
            B2[i * N + j] = dist(gen);
        }
    }

    tensor<double, 2> A(N, N);
    tensor<double, 2> B(N, N);
    tensor<double, 2> C(N, N);

    {
        auto A_view = get_view<host_tensor_view<double, 2>>(A).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                A_view(i, j) = A2[i * N + j];
            }
        }

        auto B_view = get_view<host_tensor_view<double, 2>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                B_view(i, j) = B2[i * N + j];
            }
        }
    }

    kernel simple_contraction = [A, B, C] {
        qtl::index i, j, k;

        C(i, j) = sum(k, A(i, k) * B(j, k));
    };

    simple_contraction();

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            for (long int k = 0; k < N; ++k)
            {
                C2[i * N + j] += A2[i * N + k] * B2[j * N + k];
            }
        }
    }

    double error = 0.0;

    {
        auto C_view = get_view<host_tensor_view<const double, 2>>(C).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                double diff = C_view(i, j) - C2[i * N + j];

                error += diff * diff;
            }
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-12);
}

TEST(contractions, complex_matrix_multiplication)
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

    kernel complex_mat_mul = [A, B, C] {
        qtl::index i, j, k;

        C(i, j) = sum(k, A(i, k) * B(k, j));
    };

    complex_mat_mul();

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            for (long int k = 0; k < N; ++k)
            {
                C2[i * N + j] += A2[i * N + k] * B2[k * N + j];
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

    ASSERT_NEAR(error, 0.0, 1e-12);
}

TEST(contractions, reduction_to_r1)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::vector<double> A2(N * N);
    std::vector<double> B2(N * N);
    std::vector<double> C2(N);

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            A2[i * N + j] = dist(gen);
            B2[i * N + j] = dist(gen);
        }
    }

    tensor<double, 2> A(N, N);
    tensor<double, 2> B(N, N);
    tensor<double, 1> C(N);

    {
        auto A_view = get_view<host_tensor_view<double, 2>>(A).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                A_view(i, j) = A2[i * N + j];
            }
        }

        auto B_view = get_view<host_tensor_view<double, 2>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                B_view(i, j) = B2[i * N + j];
            }
        }
    }

    kernel reduction_to_r1 = [A, B, C] {
        qtl::index i, j, k;

        C(i) = sum(j, sum(k, A(i, k) * B(j, k)));
    };

    reduction_to_r1();

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            for (long int k = 0; k < N; ++k)
            {
                C2[i] += A2[i * N + k] * B2[j * N + k];
            }
        }
    }

    double error = 0.0;

    {
        auto C_view = get_view<host_tensor_view<const double, 1>>(C).get();

        for (long int i = 0; i < N; ++i)
        {
            double diff = C_view(i) - C2[i];

            error += diff * diff;
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-12);
}

TEST(contractions, matrix_vector_product)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::vector<double> A2(N * N);
    std::vector<double> B2(N);
    std::vector<double> C2(N);

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            A2[i * N + j] = dist(gen);
        }
        B2[i] = dist(gen);
    }

    tensor<double, 2> A(N, N);
    tensor<double, 1> B(N);
    tensor<double, 1> C(N);

    {
        auto A_view = get_view<host_tensor_view<double, 2>>(A).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                A_view(i, j) = A2[i * N + j];
            }
        }

        auto B_view = get_view<host_tensor_view<double, 1>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            B_view(i) = B2[i];
        }
    }

    kernel matrix_vector_product = [A, B, C] {
        qtl::index i, j;

        C(i) = sum(j, A(i, j) * B(j));
    };

    matrix_vector_product();

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            C2[i] += A2[i * N + j] * B2[j];
        }
    }

    auto C_view = get_view<host_tensor_view<const double, 1>>(C).get();

    double error = 0.0;

    for (long int i = 0; i < N; ++i)
    {
        double diff = C_view(i) - C2[i];

        error += diff * diff;
    }

    ASSERT_NEAR(error, 0.0, 1e-12);
}

TEST(contractions, basis_change_r2)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::vector<double> A2(N * N);
    std::vector<double> B2(N * N);
    std::vector<double> C2(N * N);

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            A2[i * N + j] = dist(gen);
            B2[i * N + j] = dist(gen);
        }
    }

    qtl::index i("i");
    qtl::index j("j");
    qtl::index k("k");
    qtl::index l("l");

    tensor<double, 2> A(N, N);
    tensor<double, 2> B(N, N);
    tensor<double, 2> C(N, N);

    {
        auto A_view = get_view<host_tensor_view<double, 2>>(A).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                A_view(i, j) = A2[i * N + j];
            }
        }

        auto B_view = get_view<host_tensor_view<double, 2>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                B_view(i, j) = B2[i * N + j];
            }
        }
    }

    kernel basis_change_r2 = [A, B, C] {
        qtl::index i, j, k, l;

        C(i, j) = sum(l, sum(k, A(i, k) * B(k, l) * A(l, j)));
    };

    basis_change_r2();

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            for (long int k = 0; k < N; ++k)
            {
                for (long int l = 0; l < N; ++l)
                {
                    C2[i * N + j] += A2[i * N + k] * B2[k * N + l] * A2[l * N + j];
                }
            }
        }
    }

    double error = 0.0;

    {
        auto C_view = get_view<host_tensor_view<const double, 2>>(C).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                double diff = C_view(i, j) - C2[i * N + j];

                error += diff * diff;
            }
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-12);
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

    return hpx::init(argc, argv);
}