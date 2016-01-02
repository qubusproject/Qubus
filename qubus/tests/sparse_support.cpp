//  Copyright (c) 2015 Christopher Hinz
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <qbb/qubus/qubus.hpp>

#include <hpx/hpx_init.hpp>

#include <qbb/util/unused.hpp>

#include <vector>
#include <random>

#include <gtest/gtest.h>

class sparse_support : public ::testing::TestWithParam<long int>
{
};

INSTANTIATE_TEST_CASE_P(base_extents, sparse_support,
                        ::testing::Values(10l, 11l, 100l, 101l, 512l, 1234l));

TEST_P(sparse_support, sparse_matrix_vector_product)
{
    using namespace qbb::qubus;

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

    qbb::qubus::index i("i");
    qbb::qubus::index j("j");

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

    auto init_B = make_computelet()
                      .body([&](cpu_tensor_view<double, 1> B)
                            {
                                for (long int i = 0; i < N; ++i)
                                {
                                    B(i) = B2[i];
                                }
                            })
                      .finalize();

    execute(init_B, B);

    B.when_ready().wait();

    tensor_expr<double, 1> Cdef = def_tensor(i)[sum(A(i, j) * B(j), j)];

    C = Cdef;

    for (long int i = 0; i < N; ++i)
    {
        for (long int j = 0; j < N; ++j)
        {
            C2[i] += A2[i * N + j] * B2[j];
        }
    }

    double error;

    auto test = make_computelet()
                    .body([&](cpu_tensor_view<double, 1> C)
                          {
                              error = 0.0;

                              for (long int i = 0; i < N; ++i)
                              {
                                  double diff = C(i) - C2[i];

                                  error += diff * diff;
                              }
                          })
                    .finalize();

    execute(test, C);
    C.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-12);
}

TEST_P(sparse_support, sparse_matrix_matrix_product)
{
    using namespace qbb::qubus;

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

    qbb::qubus::index i("i");
    qbb::qubus::index j("j");
    qbb::qubus::index k("k");

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

    auto init_B = make_computelet()
                      .body([&](cpu_tensor_view<double, 2> B)
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

    execute(init_B, B);

    B.when_ready().wait();

    tensor_expr<double, 2> Cdef = def_tensor(i, k)[sum(A(i, j) * B(j, k), j)];

    C = Cdef;

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

    double error;

    auto test = make_computelet()
                    .body([&](cpu_tensor_view<double, 2> C)
                          {
                              error = 0.0;

                              for (long int i = 0; i < N; ++i)
                              {
                                  for (long int j = 0; j < N; ++j)
                                  {
                                      double diff = C(i, j) - C2[i * N + j];

                                      error += diff * diff;
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