#include <qubus/qtl/all.hpp>
#include <qubus/qubus.hpp>

#include <hpx/hpx_init.hpp>

#include <iostream>

int hpx_main(int argc, char** argv)
{
    using namespace qubus;

    qubus::init(argc, argv);

    constexpr long int N = 10;

    qtl::tensor<double, 1> A(N);
    qtl::tensor<double, 1> B(N);
    qtl::tensor<double, 1> C(N);

    {
        auto A_view = get_view<host_tensor_view<double, 1>>(A).get();

        for (long int i = 0; i < N; ++i)
        {
            A_view(i) = i;
        }

        auto B_view = get_view<host_tensor_view<double, 1>>(B).get();

        for (long int i = 0; i < N; ++i)
        {
            B_view(i) = 42 * i;
        }
    }

    static const qtl::kernel vec_add = [A, B, C] {
        qtl::index i;

        C(i) = A(i) + B(i);
    };

    vec_add();

    {
        auto C_view = get_view<host_tensor_view<const double, 1>>(C).get();

        for (long int i = 0; i < N; ++i)
        {
            std::cout << C_view(i) << '\n';
        }
    }

    qubus::finalize();

    return hpx::finalize();
}

int main(int argc, char** argv)
{
    return hpx::init(argc, argv, qubus::get_hpx_config());
}
