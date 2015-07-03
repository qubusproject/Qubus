#include <qbb/kubus/runtime.hpp>

#include <qbb/kubus/tensor_variable.hpp>

#include <qbb/kubus/user_defined_plan.hpp>

#include <hpx/hpx_init.hpp>

#include <qbb/util/unused.hpp>

#include <vector>
#include <random>

#include <gtest/gtest.h>

TEST(multi_indices, simple_expr)
{
    using namespace qbb::kubus;

    long int N = 100;

    qbb::kubus::multi_index<2> ij("ij");

    tensor<double, 2> A(N, N);

    tensor_expr<double, 2> Adef = def_tensor(ij)[0];

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

TEST(multi_indices, index_splitting)
{
    using namespace qbb::kubus;

    long int N = 100;

    qbb::kubus::index i("i");
    qbb::kubus::index j("j");
    qbb::kubus::multi_index<2> ij({i, j}, "ij");

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

    execute(init_A, A);

    tensor<double, 2> R(N, N);

    tensor_expr<double, 2> Rdef = def_tensor(ij)[A(i, j)];

    R = Rdef;

    double error;

    auto test = make_plan()
            .body([&](cpu_tensor_view<double, 2> R)
                  {
                      error = 0.0;

                      for (long int i = 0; i < N; ++i)
                      {
                          for (long int j = 0; j < N; ++j)
                          {
                              double diff = R(i, j) - A2[i * N + j];

                              error += diff * diff;
                          }
                      }
                  })
            .finalize();

    execute(test, R);
    R.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-14);
}

// TODO: Should this work?
/*TEST(multi_indices, reverse_index_splitting)
{
    using namespace qbb::kubus;

    long int N = 100;

    qbb::kubus::index i("i");
    qbb::kubus::index j("j");
    qbb::kubus::multi_index<2> ij({i, j}, "ij");

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

    execute(init_A, A);

    tensor<double, 2> R(N, N);

    tensor_expr<double, 2> Rdef = def_tensor(i, j)[A(ij)];

    R = Rdef;

    double error;

    auto test = make_plan()
            .body([&](cpu_tensor_view<double, 2> R)
                  {
                      error = 0.0;

                      for (long int i = 0; i < N; ++i)
                      {
                          for (long int j = 0; j < N; ++j)
                          {
                              double diff = R(i, j) - A2[i * N + j];

                              error += diff * diff;
                          }
                      }
                  })
            .finalize();

    execute(test, R);
    R.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-14);
}*/

TEST(multi_indices, multi_sum)
{
    using namespace qbb::kubus;

    long int N = 100;

    qbb::kubus::multi_index<2> ij("ij");
    qbb::kubus::index k("k");

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

    auto init_A = make_plan()
                      .body([&](cpu_tensor_view<double, 3> A)
                            {
                                for (long int i = 0; i < N; ++i)
                                {
                                    for (long int j = 0; j < N; ++j)
                                    {
                                        for (long int k = 0; k < N; ++k)
                                        {
                                            A(i, j, k) = A2[i * N * N + j * N + k];
                                        }
                                    }
                                }
                            })
                      .finalize();

    execute(init_A, A);

    tensor<double, 1> R(N);

    tensor_expr<double, 1> Rdef = def_tensor(k)[sum(A(ij, k), ij)];

    R = Rdef;

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

    double error;

    auto test = make_plan()
                    .body([&](cpu_tensor_view<double, 1> R)
                          {
                              error = 0.0;

                              for (long int i = 0; i < N; ++i)
                              {
                                  double diff = R(i) - R2[i];

                                  error += diff * diff;
                              }
                          })
                    .finalize();

    execute(test, R);
    R.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(multi_indices, multi_sum_index_splitting)
{
    using namespace qbb::kubus;

    long int N = 100;

    qbb::kubus::index i("i");
    qbb::kubus::index j("j");
    qbb::kubus::multi_index<2> ij({i, j}, "ij");
    qbb::kubus::index k("k");

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

    auto init_A = make_plan()
            .body([&](cpu_tensor_view<double, 3> A)
                  {
                      for (long int i = 0; i < N; ++i)
                      {
                          for (long int j = 0; j < N; ++j)
                          {
                              for (long int k = 0; k < N; ++k)
                              {
                                  A(i, j, k) = A2[i * N * N + j * N + k];
                              }
                          }
                      }
                  })
            .finalize();

    execute(init_A, A);

    tensor<double, 1> R(N);

    tensor_expr<double, 1> Rdef = def_tensor(k)[sum(A(i, j, k), ij)];

    R = Rdef;

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

    double error;

    auto test = make_plan()
            .body([&](cpu_tensor_view<double, 1> R)
                  {
                      error = 0.0;

                      for (long int i = 0; i < N; ++i)
                      {
                          double diff = R(i) - R2[i];

                          error += diff * diff;
                      }
                  })
            .finalize();

    execute(test, R);
    R.when_ready().wait();

    ASSERT_NEAR(error, 0.0, 1e-14);
}

int hpx_main(int argc, char** argv)
{
    qbb::kubus::init(argc, argv);

    auto result = RUN_ALL_TESTS();

    hpx::finalize();

    return result;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return hpx::init(argc, argv);
}
