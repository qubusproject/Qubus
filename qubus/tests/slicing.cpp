#include <qubus/qubus.hpp>

#include <qubus/IR/parsing.hpp>

#include <qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <gtest/gtest.h>

#include <fstream>

std::string read_code(const std::string& filepath)
{
    std::ifstream fin(filepath);

    auto first = std::istreambuf_iterator<char>(fin);
    auto last = std::istreambuf_iterator<char>();

    std::string code(first, last);

    return code;
}

TEST(slicing, simple_slice_base)
{
    using namespace qubus;

    auto runtime = qubus::get_runtime();

    auto obj_factory = runtime.get_object_factory();

    auto x = obj_factory.create_array(qubus::types::double_{}, {30});
    auto r = obj_factory.create_scalar(qubus::types::double_{});

    {
        auto x_view = qubus::get_view<qubus::array<double, 1>>(x, qubus::writable, qubus::arch::host).get();

        for (long int i = 0; i < 10; ++i)
        {
            x_view(i) = 1.0;
        }

        for (long int i = 10; i < 20; ++i)
        {
            x_view(i) = 10.0;
        }

        for (long int i = 20; i < 30; ++i)
        {
            x_view(i) = 42.0;
        }
    }

    auto code = read_code("samples/slicing");

    auto mod = qubus::parse_qir(std::move(code));

    runtime.get_module_library().add(std::move(mod)).get();

    qubus::kernel_arguments args;

    args.push_back_arg(x);
    args.push_back_result(r);

    runtime.execute(qubus::symbol_id("slicing.calc"), args).get();

    {
        auto r_view = qubus::get_view<qubus::scalar<double>>(r, qubus::immutable, qubus::arch::host).get();

        auto result = r_view.get();

        ASSERT_NEAR(result, 100.0, 1e-8);
    }
}

TEST(slicing, simple_slice)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 100;

    tensor<double, 1> A(N / 2);
    tensor<double, 1> B(N);

    {
        auto B_view = get_view(B, qubus::writable, qubus::arch::host).get();

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

        A(i) = B(qtl::range(0, N, 2))(i);
    };

    simple_slice();

    double error = 0.0;

    {
        auto A_view = get_view(A, qubus::immutable, qubus::arch::host).get();

        for (long int i = 0; i < N / 2; ++i)
        {
            double diff = A_view(i) - 42.0;

            error += diff * diff;
        }
    }

    ASSERT_NEAR(error, 0.0, 1e-14);
}

// Recursive slicing is currently unimplemented and pending a major rework.
TEST(slicing, DISABLED_recursive_slicing)
{
    using namespace qubus;
    using namespace qtl;

    long int N = 16;

    tensor<double, 1> A(N / 8);
    tensor<double, 1> B(N);

    {
        auto B_view = get_view(B, qubus::writable, qubus::arch::host).get();

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

        A(i) = B(qtl::range(0, N, 2))(qtl::range(N / 4, N / 2, 2))(i);
    };

    recursive_slicing();

    double error = 0.0;

    {
        auto A_view = get_view(A, qubus::immutable, qubus::arch::host).get();

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
