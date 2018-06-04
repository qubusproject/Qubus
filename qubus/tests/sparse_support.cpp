/*#include <qubus/qubus.hpp>

#include <qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <random>
#include <vector>

#include <gtest/gtest.h>

class sparse_support : public ::testing::TestWithParam<long int>
{
};

INSTANTIATE_TEST_CASE_P(base_extents, sparse_support,
                        ::testing::Values(10l, 11l, 100l, 101l, 512l, 1234l));

TEST_P(sparse_support, DISABLED_sparse_matrix_vector_product)
{
    using namespace qubus;
    using namespace qtl;

    long int N = GetParam();

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

    kernel spmv = [A, B, C] {
        qtl::index i, j;

        C(i) = sum(j, A(i, j) * B(j));
    };

    {
        auto B_view = get_view(B, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            B_view(i) = B2[i];
        }
    }

    spmv();

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            C2[i] += A2[i * N + j] * B2[j];
        }
    }

    double error = 0.0;

    {
        auto C_view = get_view(C, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            double diff = C_view(i) - C2[i];

            error += diff * diff;
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-12);
}

TEST_P(sparse_support, DISABLED_sparse_matrix_matrix_product)
{
    using namespace qubus;
    using namespace qtl;

    long int N = GetParam();

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
            A2[i * N + j] = 0;
            B2[i * N + j] = dist(gen);
        }
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
    tensor<double, 2> B(N, N);
    tensor<double, 2> C(N, N);

    {
        auto B_view = get_view(B, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                B_view(i, j) = B2[i * N + j];
            }
        }
    }

    kernel spmm = [A, B, C] {
        qtl::index i, j, k;

        C(i, k) = sum(j, A(i, j) * B(j, k));
    };

    spmm();

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            for (long int k = 0; k < N; ++k)
            {
                C2[i * N + k] += A2[i * N + j] * B2[j * N + k];
            }
        }
    }

    double error = 0.0;

    {
        auto C_view = get_view(C, qubus::immutable, qubus::arch::host).get();

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

    hpx::resource::partitioner rp(argc, argv, qubus::get_hpx_config(),
                                  hpx::resource::partitioner_mode::mode_allow_oversubscription);

    qubus::setup(rp);

    return hpx::init();
}*/