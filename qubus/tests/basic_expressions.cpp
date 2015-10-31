//  Copyright (c) 2015 Christopher Hinz
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <qbb/qubus/qubus.hpp>

#include <hpx/hpx_init.hpp>

#include <qbb/util/unused.hpp>

#include <vector>
#include <random>
#include <complex>

#include <gtest/gtest.h>

TEST(basic_expressions, constant_expr)
{
    using namespace qbb::qubus;

    long int N = 100;

    qbb::qubus::index i("i");
    qbb::qubus::index j("j");

    tensor<double, 2> A(N, N);

    tensor_expr<double, 2> Adef = def_tensor(i, j)[0];

    A = Adef;

    double error;

    auto test = make_plan()
                    .body([&](cpu_tensor_view<double, 2> A)
                          {
                              error = 0.0;

                              for (long int i = 0; i < N; ++i)
                              {
                                  for (long int j = 0; j < N; ++j)
                                  {
                                      double diff = A(i, j) - 0.0;

                                      error += diff * diff;
                                  }
                              }
                          })
                    .finalize();

    execute(test, A);
    A.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(basic_expressions, complex_addition)
{
    using namespace qbb::qubus;

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

    qbb::qubus::index i("i");

    tensor<std::complex<double>, 1> A(N);
    tensor<std::complex<double>, 1> B(N);
    tensor<std::complex<double>, 1> C(N);

    auto init_A = make_plan()
            .body([&](cpu_tensor_view<std::complex<double>, 1> A)
                  {
                      for (long int i = 0; i < N; ++i)
                      {
                          A(i) = A2[i];
                      }
                  })
            .finalize();

    auto init_B = make_plan()
            .body([&](cpu_tensor_view<std::complex<double>, 1> B)
                  {
                      for (long int i = 0; i < N; ++i)
                      {
                          B(i) = B2[i];
                      }
                  })
            .finalize();

    execute(init_A, A);
    execute(init_B, B);

    tensor_expr<std::complex<double>, 1> Cdef = def_tensor(i)[A(i) + B(i)];

    C = Cdef;

    for (long int i = 0; i < N; ++i)
    {
        C2[i] = A2[i] + B2[i];
    }

    double error;

    auto test = make_plan()
            .body([&](cpu_tensor_view<std::complex<double>, 1> C)
                  {
                      error = 0.0;

                      for (long int i = 0; i < N; ++i)
                      {
                          std::complex<double> diff = C(i) - C2[i];

                          error += std::norm(diff);
                      }
                  })
            .finalize();

    execute(test, C);
    C.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(basic_expressions, complex_substraction)
{
    using namespace qbb::qubus;

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

    qbb::qubus::index i("i");

    tensor<std::complex<double>, 1> A(N);
    tensor<std::complex<double>, 1> B(N);
    tensor<std::complex<double>, 1> C(N);

    auto init_A = make_plan()
            .body([&](cpu_tensor_view<std::complex<double>, 1> A)
                  {
                      for (long int i = 0; i < N; ++i)
                      {
                          A(i) = A2[i];
                      }
                  })
            .finalize();

    auto init_B = make_plan()
            .body([&](cpu_tensor_view<std::complex<double>, 1> B)
                  {
                      for (long int i = 0; i < N; ++i)
                      {
                          B(i) = B2[i];
                      }
                  })
            .finalize();

    execute(init_A, A);
    execute(init_B, B);

    tensor_expr<std::complex<double>, 1> Cdef = def_tensor(i)[A(i) - B(i)];

    C = Cdef;

    for (long int i = 0; i < N; ++i)
    {
        C2[i] = A2[i] - B2[i];
    }

    double error;

    auto test = make_plan()
            .body([&](cpu_tensor_view<std::complex<double>, 1> C)
                  {
                      error = 0.0;

                      for (long int i = 0; i < N; ++i)
                      {
                          std::complex<double> diff = C(i) - C2[i];

                          error += std::norm(diff);
                      }
                  })
            .finalize();

    execute(test, C);
    C.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(basic_expressions, complex_multiplication)
{
    using namespace qbb::qubus;

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

    qbb::qubus::index i("i");

    tensor<std::complex<double>, 1> A(N);
    tensor<std::complex<double>, 1> B(N);
    tensor<std::complex<double>, 1> C(N);

    auto init_A = make_plan()
                      .body([&](cpu_tensor_view<std::complex<double>, 1> A)
                            {
                                for (long int i = 0; i < N; ++i)
                                {
                                    A(i) = A2[i];
                                }
                            })
                      .finalize();

    auto init_B = make_plan()
                      .body([&](cpu_tensor_view<std::complex<double>, 1> B)
                            {
                                for (long int i = 0; i < N; ++i)
                                {
                                    B(i) = B2[i];
                                }
                            })
                      .finalize();

    execute(init_A, A);
    execute(init_B, B);

    tensor_expr<std::complex<double>, 1> Cdef = def_tensor(i)[A(i) * B(i)];

    C = Cdef;

    for (long int i = 0; i < N; ++i)
    {
        C2[i] = A2[i] * B2[i];
    }

    double error;

    auto test = make_plan()
                    .body([&](cpu_tensor_view<std::complex<double>, 1> C)
                          {
                              error = 0.0;

                              for (long int i = 0; i < N; ++i)
                              {
                                  std::complex<double> diff = C(i) - C2[i];

                                  error += std::norm(diff);
                              }
                          })
                    .finalize();

    execute(test, C);
    C.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(basic_expressions, complex_division)
{
    using namespace qbb::qubus;

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

    qbb::qubus::index i("i");

    tensor<std::complex<double>, 1> A(N);
    tensor<std::complex<double>, 1> B(N);
    tensor<std::complex<double>, 1> C(N);

    auto init_A = make_plan()
            .body([&](cpu_tensor_view<std::complex<double>, 1> A)
                  {
                      for (long int i = 0; i < N; ++i)
                      {
                          A(i) = A2[i];
                      }
                  })
            .finalize();

    auto init_B = make_plan()
            .body([&](cpu_tensor_view<std::complex<double>, 1> B)
                  {
                      for (long int i = 0; i < N; ++i)
                      {
                          B(i) = B2[i];
                      }
                  })
            .finalize();

    execute(init_A, A);
    execute(init_B, B);

    tensor_expr<std::complex<double>, 1> Cdef = def_tensor(i)[A(i) / B(i)];

    C = Cdef;

    for (long int i = 0; i < N; ++i)
    {
        C2[i] = A2[i] / B2[i];
    }

    double error;

    auto test = make_plan()
            .body([&](cpu_tensor_view<std::complex<double>, 1> C)
                  {
                      error = 0.0;

                      for (long int i = 0; i < N; ++i)
                      {
                          std::complex<double> diff = C(i) - C2[i];

                          error += std::norm(diff);
                      }
                  })
            .finalize();

    execute(test, C);
    C.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-14);
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
