//  Copyright (c) 2012-2014 Christopher Hinz
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_DYN_LINK

#include <qbb/kubus/runtime.hpp>

#include <qbb/kubus/compilation_cache.hpp>

#include <qbb/kubus/tensor_variable.hpp>

#include <qbb/kubus/user_defined_plan.hpp>

#include <hpx/hpx_init.hpp>

#include <vector>
#include <random>

#define BOOST_TEST_MODULE "simple contraction unit tests"

#include "kubus_unit_test.hpp"

BOOST_AUTO_TEST_SUITE(simple_contraction_suite)

BOOST_AUTO_TEST_CASE(simple_contraction)
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

    for (std::size_t i = 0; i < N; ++i)
    {
        for (std::size_t j = 0; j < N; ++j)
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
    
    BOOST_CHECK_SMALL(error, 1e-12);
}

BOOST_AUTO_TEST_SUITE_END()
