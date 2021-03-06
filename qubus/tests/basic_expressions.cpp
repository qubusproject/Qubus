#include <qubus/qubus.hpp>

#include <qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <qubus/util/unused.hpp>

#include <complex>
#include <random>
#include <vector>

#include <gtest/gtest.h>

TEST(basic_expressions, constant_expr)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    tensor<double, 2> A(N, N);

    kernel const_kernel = [A] {
        qtl::index i, j;
        A(i, j) = 42;
    };

    const_kernel();

    double error = 0.0;

    {
        auto A_view = get_view(A, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            for (long int j = 0; j < N; ++j)
            {
                double diff = A_view(i, j) - 42.0;

                error += diff * diff;
            }
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(basic_expressions, complex_addition)
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
        auto A_view = get_view(A, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            A_view(i) = A2[i];
        }

        auto B_view = get_view(B, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            B_view(i) = B2[i];
        }
    }

    kernel vec_add = [A, B, C] {
        qtl::index i;
        C(i) = A(i) + B(i);
    };

    vec_add();

    for (long int i = 0; i < N; ++i)
    {
        C2[i] = A2[i] + B2[i];
    }

    double error = 0.0;

    {
        auto C_view = get_view(C, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            std::complex<double> diff = C_view(i) - C2[i];

            error += std::norm(diff);
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(basic_expressions, complex_substraction)
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
        auto A_view = get_view(A, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            A_view(i) = A2[i];
        }

        auto B_view = get_view(B, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            B_view(i) = B2[i];
        }
    }

    kernel vec_sub = [A, B, C] {
        qtl::index i;
        C(i) = A(i) - B(i);
    };

    vec_sub();

    for (long int i = 0; i < N; ++i)
    {
        C2[i] = A2[i] - B2[i];
    }

    double error = 0.0;

    {
        auto C_view = get_view(C, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            std::complex<double> diff = C_view(i) - C2[i];

            error += std::norm(diff);
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(basic_expressions, complex_multiplication)
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
        auto A_view = get_view(A, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            A_view(i) = A2[i];
        }

        auto B_view = get_view(B, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            B_view(i) = B2[i];
        }
    }

    kernel vec_mul = [A, B, C] {
        qtl::index i;
        C(i) = A(i) * B(i);
    };

    vec_mul();

    for (long int i = 0; i < N; ++i)
    {
        C2[i] = A2[i] * B2[i];
    }

    double error = 0.0;

    {
        auto C_view = get_view(C, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            std::complex<double> diff = C_view(i) - C2[i];

            error += std::norm(diff);
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

TEST(basic_expressions, complex_division)
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
        auto A_view = get_view(A, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            A_view(i) = A2[i];
        }

        auto B_view = get_view(B, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < N; ++i)
        {
            B_view(i) = B2[i];
        }
    }

    kernel vec_div = [A, B, C] {
        qtl::index i;
        C(i) = A(i) / B(i);
    };

    vec_div();

    for (long int i = 0; i < N; ++i)
    {
        C2[i] = A2[i] / B2[i];
    }

    double error = 0.0;

    {
        auto C_view = get_view(C, qubus::immutable, qubus::arch::host).get();

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
