#include <qubus/qubus.hpp>

#include <qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

TEST(slicing, simple_slice)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    tensor<double, 1> A(N / 2);
    tensor<double, 1> B(N);

    {
        auto B_view = get_view<host_tensor_view<double, 1>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            if (i % 2 == 0)
            {
                B_view(i) = 42;
            }
            else
            {
                B_view(i) = -1;
            }
        }
    }

    kernel simple_slice = [A, B, N] {
        qtl::index i;

        A(i) = B(range(0, N, 2))(i);
    };

    simple_slice();

    double error = 0.0;

    {
        auto A_view = get_view<host_tensor_view<const double, 1>>(A).get();

        for (long int i = 0; i < N / 2; ++i)
        {
            double diff = A_view(i) - 42.0;

            error += diff * diff;
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(slicing, recursive_slicing)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 16;

    tensor<double, 1> A(N / 8);
    tensor<double, 1> B(N);

    {
        auto B_view = get_view<host_tensor_view<double, 1>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            if (i % 2 == 0)
            {
                if (i / 2 >= N / 4)
                {
                    if (i / 2 % 2 == 0)
                    {
                        B_view(i) = 42;
                    }
                    else
                    {
                        B_view(i) = -3;
                    }
                }
                else
                {
                    B_view(i) = -2;
                }
            }
            else
            {
                B_view(i) = -1;
            }
        }
    }

    kernel recursive_slicing = [A, B, N] {
        qtl::index i;

        A(i) = B(range(0, N, 2))(range(N / 4, N / 2, 2))(i);
    };

    recursive_slicing();

    double error = 0.0;

    {
        auto A_view = get_view<host_tensor_view<const double, 1>>(A).get();

        for (long int i = 0; i < N / 8; ++i)
        {
            double diff = A_view(i) - 42.0;

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
