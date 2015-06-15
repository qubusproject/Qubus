//  Copyright (c) 2015 Christopher Hinz
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


TEST(basic_expressions, constant_expr)
{
using namespace qbb::kubus;

long int N = 100;

qbb::kubus::index i("i");
qbb::kubus::index j("j");

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

