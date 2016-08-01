#include <hpx/config.hpp>

#include <qbb/qubus/qubus.hpp>
#include <qbb/qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>

#include <cstring>
#include <fstream>
#include <functional>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

#include <random>

// for tests
#include <chrono>

template <typename F, typename OnStop>
double run_benchmark(F f, OnStop on_stop)
{
    long int min_samples = 100;

    double min_time = 1.0;

    double duration = 0.0;
    long int samples = 0;

    do
    {
        auto start = std::chrono::high_resolution_clock::now();

        for (long int i = 0; i < min_samples; ++i)
        {
            f();
        }

        on_stop();

        auto end = std::chrono::high_resolution_clock::now();

        duration += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        samples += min_samples;
    } while (duration < min_time * 1000.0);

    auto avg_time = duration / (1000.0 * static_cast<double>(samples));

    return avg_time;
}

using namespace qbb::qubus;

int hpx_main(int argc, char** argv)
{
    qbb::qubus::init(argc, argv);

    std::ofstream bench_data("bench_data.dat");

    qtl::tensor_expr<double, 1> init = [](qtl::index i) { return 42; };

    for (long int i = 1; i < 100; ++i)
    {
        long int N = i * 1000000;

        bench_data << N << "   ";

        {
            qtl::tensor<double, 1> v(N);

            v = init;

            v.when_ready().wait();

            auto result = run_benchmark([&] { v = init; }, [&] { v.when_ready().wait(); });

            bench_data << result << "   ";
        }

        bench_data << std::endl;
    }

    return hpx::finalize();
}

int main(int argc, char** argv)
{
    return hpx::init(argc, argv);
}
