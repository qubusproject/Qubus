#include <qubus/qubus.hpp>

#include <qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

void kernel1_host(qubus::host_tensor_view<double, 1> A)
{
    for (qubus::util::index_t i = 0; i < A.extent(0); ++i)
    {
        A(i) = 42;
    }
}

qubus::foreign_kernel<qubus::qtl::tensor<double, 1>> kernel1("kernel1");

QUBUS_ADD_FOREIGN_KERNEL_VERSION(kernel1, qubus::architectures::host, kernel1_host);

void kernel2_host(qubus::host_tensor_view<const double, 1> A, qubus::host_tensor_view<double, 1> B)
{
    for (qubus::util::index_t i = 0; i < A.extent(0); ++i)
    {
        B(i) = A(i);
    }
}

qubus::foreign_kernel<const qubus::qtl::tensor<double, 1>, qubus::qtl::tensor<double, 1>> kernel2("kernel2");

QUBUS_ADD_FOREIGN_KERNEL_VERSION(kernel2, qubus::architectures::host, kernel2_host);

TEST(foreign_kernels, constant_expr)
{
    using namespace qubus;
    using namespace qtl;

    util::index_t N = 100;

    tensor<double, 1> A(N);

    kernel1(A);

    double error = 0.0;

    {
        auto A_view = get_view<host_tensor_view<const double, 1>>(A).get();

        for (util::index_t i = 0; i < N; ++i)
        {
            double diff = A_view(i) - 42.0;

            error += diff * diff;
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(foreign_kernels, copy)
{
    using namespace qubus;
    using namespace qtl;

    util::index_t N = 100;

    tensor<double, 1> A(N);
    tensor<double, 1> B(N);

    {
        auto A_view = get_view<host_tensor_view<double, 1>>(A).get();

        for (util::index_t i = 0; i < N; ++i)
        {
            A_view(i) = 42;
        }
    }

    kernel2(A, B);

    double error = 0.0;

    {
        auto A_view = get_view<host_tensor_view<const double, 1>>(A).get();
        auto B_view = get_view<host_tensor_view<const double, 1>>(B).get();

        for (util::index_t i = 0; i < N; ++i)
        {
            double diff = B_view(i) - A_view(i);

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

    return hpx::init(argc, argv);
}
