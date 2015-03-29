//  Copyright (c) 2012-2014 Christopher Hinz
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <qbb/kubus/runtime.hpp>

#include <qbb/kubus/tensor_variable.hpp>

#include <qbb/kubus/user_defined_plan.hpp>

#include <hpx/hpx_init.hpp>

#include <qbb/util/unused.hpp>

#include <vector>
#include <random>

#include <gtest/gtest.h>


TEST(contractions, simple_contraction)
{
    using namespace qbb::kubus;
    
    qbb::kubus::init();
    
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

    qbb::kubus::index i("i");
    qbb::kubus::index j("j");
    qbb::kubus::index k("k");

    tensor<double, 2> A(N, N);
    tensor<double, 2> B(N, N);
    tensor<double, 2> C(N, N);

    auto init_A = make_plan()
                      .body([&](cpu_tensor_view<double, 2> A)
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

    execute(init_A, A);
    execute(init_B, B);

    tensor_expr<double, 2> Cdef = def_tensor(i, j)[sum(A(i, k) * B(j, k), k)];
    
    C = Cdef;

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
    
    double error;
    
    auto test = make_plan()
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

TEST(contractions, reduction_to_r1)
{
    using namespace qbb::kubus;
    
    qbb::kubus::init();
    
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

    qbb::kubus::index i("i");
    qbb::kubus::index j("j");
    qbb::kubus::index k("k");

    tensor<double, 2> A(N, N);
    tensor<double, 2> B(N, N);
    tensor<double, 1> C(N);

    auto init_A = make_plan()
                      .body([&](cpu_tensor_view<double, 2> A)
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

    execute(init_A, A);
    execute(init_B, B);

    tensor_expr<double, 1> Cdef = def_tensor(i)[sum(sum(A(i, k) * B(j, k), k), j)];
    
    C = Cdef;

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
    
    double error;
    
    auto test = make_plan()
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

TEST(contractions, matrix_vector_product)
{
    using namespace qbb::kubus;
    
    qbb::kubus::init();
    
    long int N = 100;

    std::vector<double> A2(N * N);
    std::vector<double> B2(N );
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

    qbb::kubus::index i("i");
    qbb::kubus::index j("j");

    tensor<double, 2> A(N, N);
    tensor<double, 1> B(N);
    tensor<double, 1> C(N);

    auto init_A = make_plan()
                      .body([&](cpu_tensor_view<double, 2> A)
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
                      .body([&](cpu_tensor_view<double, 1> B)
                            {
                                for (long int i = 0; i < N; ++i)
                                {
                                    B(i) = B2[i];
                                }
                            })
                      .finalize();

    execute(init_A, A);
    execute(init_B, B);

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
    
    auto test = make_plan()
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

TEST(contractions, basis_change_r2)
{
    using namespace qbb::kubus;
    
    qbb::kubus::init();
    
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

    qbb::kubus::index i("i");
    qbb::kubus::index j("j");
    qbb::kubus::index k("k");
    qbb::kubus::index l("l");

    tensor<double, 2> A(N, N);
    tensor<double, 2> B(N, N);
    tensor<double, 2> C(N, N);

    auto init_A = make_plan()
                      .body([&](cpu_tensor_view<double, 2> A)
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

    execute(init_A, A);
    execute(init_B, B);

    tensor_expr<double, 2> Cdef = def_tensor(i, j)[sum(sum(A(i, k) * B(k, l) * A(l, j), k), l)];
    
    C = Cdef;

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
    
    double error;
    
    auto test = make_plan()
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

int hpx_main(int QBB_UNUSED(argc), char** QBB_UNUSED(argv))
{
    auto result = RUN_ALL_TESTS();

    hpx::finalize();
    
    return result;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return hpx::init(argc, argv);
}