#include <hpx/config.hpp>

#include <iostream>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Sparse>

#include <qubus/qubus.hpp>

#include <hpx/hpx_init.hpp>

#include <memory>
#include <cstring>
#include <utility>
#include <vector>
#include <functional>
#include <numeric>
#include <fstream>

#include <random>

// for tests
#include <chrono>

template<typename F, typename OnStop>
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
    } while (duration < min_time*1000.0);

    auto avg_time = duration / (1000.0 * static_cast<double>(samples));

    return avg_time;
}

using namespace qubus;

int hpx_main(int argc, char** argv)
{
    qubus::init(argc, argv);

    std::ofstream bench_data("bench_data.dat");

    for (long int i = 1; i < 21; ++i)
    {
        long int N = i * 10000;

        //long int M = (0.01 * N) * N;

        long int M = 100 * N;

        std::random_device dev;

        std::mt19937_64 eng(dev());

        std::uniform_int_distribution<long int> dist(0, N-1);
        std::uniform_int_distribution<long int> dist2(0, 1000);

        std::vector<Eigen::Triplet<double>> nonzeros;

        for (long int j = 0; j < M; ++j)
        {
            nonzeros.push_back(Eigen::Triplet<double>(dist(eng), dist2(eng), 1.0));
        }

        /*for (long int j = 0; j < N; ++j)
        {
            nonzeros.push_back(Eigen::Triplet<double>(j, j, 1.0));
        }*/

        bench_data << N << "   ";

        {
            qubus::assembly_tensor<double, 2> A_assembly(N, N);

            for (const auto& nonzero : nonzeros)
            {
                A_assembly.add_nonzero({nonzero.row(), nonzero.col()}, nonzero.value());
            }

            qubus::index i("i");
            qubus::index j("j");
            qubus::index k("k");

            sparse_tensor<double, 2> A = std::move(A_assembly);

            tensor<double, 1> B(N);
            tensor<double, 1> C(N);

            tensor_expr<double, 1> zeros = def_tensor(i)[0];

            tensor_expr<double, 1> Cdef = def_tensor(i)[sum(A(i, k) * B(k), k)];

            B = zeros;

            C = Cdef;
            C.when_ready().wait();

            auto result = run_benchmark([&]
                                        {
                                            C = Cdef;
                                        }, [&]{ C.when_ready().wait(); });

            bench_data << result << "   ";
        }

        {
            Eigen::SparseMatrix<double> A(N, N);
            A.setFromTriplets(nonzeros.begin(), nonzeros.end());
            Eigen::VectorXd B = Eigen::VectorXd::Zero(N);
            Eigen::VectorXd C = Eigen::VectorXd::Zero(N);

            auto result = run_benchmark([&]
                                        {
                                            C.noalias() = A * B;
                                        }, []{});

            bench_data << result << "   ";
        }

        bench_data << std::endl;
    }

    return hpx::finalize();
}

int main(int argc, char** argv)
{
    return hpx::init(argc, argv, qubus::get_hpx_config());
}
