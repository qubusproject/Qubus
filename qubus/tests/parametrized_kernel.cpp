#include <qubus/qubus.hpp>

#include <qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <qubus/util/unused.hpp>

#include <complex>
#include <random>
#include <vector>

#include <gtest/gtest.h>

TEST(parametrized_kernel, vector_add)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    std::vector<std::complex<double>> A2(N);
    std::vector<std::complex<double>> B2(N);
    std::vector<std::complex<double>> C2(N);

    for (long int i = 0; i < N; ++i)
    {
        A2[i] = std::complex<double>(dist(gen), dist(gen));
        B2[i] = std::complex<double>(dist(gen), dist(gen));
    }

    tensor<std::complex<double>, 1> A(N);
    tensor<std::complex<double>, 1> B(N);
    tensor<std::complex<double>, 1> C(N);

    {
        auto A_view = get_view<host_tensor_view<std::complex<double>, 1>>(A).get();

        for (long int i = 0; i < N; ++i)
        {
            A_view(i) = A2[i];
        }

        auto B_view = get_view<host_tensor_view<std::complex<double>, 1>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            B_view(i) = B2[i];
        }
    }

    kernel vec_add = [](tensor_var<std::complex<double>, 1> A,
                        tensor_var<std::complex<double>, 1> B,
                        tensor_var<std::complex<double>, 1> C) {
        qtl::index i;

        C(i) = A(i) + B(i);
    };

    vec_add(A, B, C);

    for (long int i = 0; i < N; ++i)
    {
        C2[i] = A2[i] + B2[i];
    }

    double error = 0.0;

    {
        auto C_view = get_view<host_tensor_view<const std::complex<double>, 1>>(C).get();

        for (long int i = 0; i < N; ++i)
        {
            std::complex<double> diff = C_view(i) - C2[i];

            error += std::norm(diff);
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(parametrized_kernel, sparse_matrix_vector_product)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100l;

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
            A2[i * N + j] = 0;
        }
        B2[i] = dist(gen);
    }

    assembly_tensor<double, 2> A_assemb(N, N);

    long int N_nonzero = N * N * 0.1;

    std::uniform_int_distribution<long int> idx_dist(0, N - 1);

    for (long int k = 0; k < N_nonzero; ++k)
    {
        long int i = idx_dist(gen);
        long int j = idx_dist(gen);

        double value = dist(gen);

        A2[i * N + j] += value;
        A_assemb.add_nonzero({{i, j}}, value);
    }

    sparse_tensor<double, 2> A = std::move(A_assemb);
    tensor<double, 1> B(N);
    tensor<double, 1> C(N);

    {
        auto B_view = get_view<host_tensor_view<double, 1>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            B_view(i) = B2[i];
        }
    }

    kernel spmv = [](sparse_tensor_var<double, 2> A, tensor_var<double, 1> B,
                     tensor_var<double, 1> C) {
        qtl::index i, j;

        C(i) = sum(j, A(i, j) * B(j));
    };

    spmv(A, B, C);

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            C2[i] += A2[i * N + j] * B2[j];
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

TEST(parametrized_kernel, partial_parameterization)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    std::vector<std::complex<double>> A2(N);
    std::vector<std::complex<double>> B2(N);
    std::vector<std::complex<double>> C2(N);

    for (long int i = 0; i < N; ++i)
    {
        A2[i] = std::complex<double>(dist(gen), dist(gen));
        B2[i] = std::complex<double>(dist(gen), dist(gen));
    }

    tensor<std::complex<double>, 1> A(N);
    tensor<std::complex<double>, 1> B(N);
    tensor<std::complex<double>, 1> C(N);

    {
        auto A_view = get_view<host_tensor_view<std::complex<double>, 1>>(A).get();

        for (long int i = 0; i < N; ++i)
        {
            A_view(i) = A2[i];
        }

        auto B_view = get_view<host_tensor_view<std::complex<double>, 1>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            B_view(i) = B2[i];
        }
    }

    kernel vec_add = [C, B](tensor_var<std::complex<double>, 1> A) {
        qtl::index i;

        C(i) = A(i) + B(i);
    };

    vec_add(A);

    for (long int i = 0; i < N; ++i)
    {
        C2[i] = A2[i] + B2[i];
    }

    double error = 0.0;

    {
        auto C_view = get_view<host_tensor_view<const std::complex<double>, 1>>(C).get();

        for (long int i = 0; i < N; ++i)
        {
            std::complex<double> diff = C_view(i) - C2[i];

            error += std::norm(diff);
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
