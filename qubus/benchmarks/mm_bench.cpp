#include <hpx/config.hpp>

#include <iostream>
#include <eigen3/Eigen/Core>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/qubus/runtime.hpp>

#include <qbb/qubus/tensor_variable.hpp>

#include <qbb/qubus/user_defined_plan.hpp>

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

using namespace qbb::qubus;

int hpx_main(int argc, char** argv)
{
    qbb::qubus::init(argc, argv);

    std::ofstream bench_data("bench_data.dat");

    for (long int i = 1; i < 11; ++i)
    {
        long int N = i * 100;

        bench_data << N << "   ";

        {
            qbb::qubus::index i("i");
            qbb::qubus::index j("j");
            qbb::qubus::index k("k");

            tensor<double, 2> A(N, N);
            tensor<double, 2> B(N, N);
            tensor<double, 2> C(N, N);

            tensor_expr<double, 2> zeros = def_tensor(i, j)[0];

            tensor_expr<double, 2> Cdef = def_tensor(i, j)[sum(A(i, k) * B(k, j), k)];

            A = zeros;
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
            std::vector<double> A(N * N);
            std::vector<double> B(N * N);
            std::vector<double> C(N * N);

            auto result = run_benchmark([&]
                          {
                for (long int i = 0; i < N; ++i)
                {
                    for (long int j = 0; j < N; ++j)
                    {
                        C[i * N + j] = 0.0;
                    }
                }

                for (long int i = 0; i < N; i += 200)
                {
                    for (long int j = 0; j < N; j += 200)
                    {
                        for (long int k = 0; k < N; k += 200)
                        {
                            for (long int ii = i; ii < i + 200; ii += 20)
                            {
                                for (long int jj = j; jj < j + 200; jj += 20)
                                {
                                    for (long int kk = k; kk < k + 200; kk += 20)
                                    {
                                        for (long int iii = ii; iii < ii + 20; iii += 2)
                                        {
                                            for (long int jjj = jj; jjj < jj + 20; jjj += 2)
                                            {
                                                double C11 = C[iii * N + jjj];
                                                double C12 = C[iii * N + jjj + 1];
                                                double C21 = C[(iii + 1) * N + jjj];
                                                double C22 = C[(iii + 1) * N + jjj + 1];

                                                for (long int kkk = kk; kkk < kk + 20; kkk += 2)
                                                {
                                                    double A11 = A[iii * N + kkk];
                                                    double A12 = A[iii * N + kkk + 1];
                                                    double A21 = A[(iii + 1) * N + kkk];
                                                    double A22 = A[(iii + 1) * N + kkk + 1];

                                                    double B11 = B[jjj * N + kkk];
                                                    double B12 = B[jjj * N + kkk + 1];
                                                    double B21 = B[(jjj + 1) * N + kkk];
                                                    double B22 = B[(jjj + 1) * N + kkk + 1];

                                                    C11 += A11 * B11;
                                                    C11 += A12 * B12;

                                                    C12 += A11 * B21;
                                                    C12 += A12 * B22;

                                                    C21 += A21 * B11;
                                                    C21 += A22 * B12;

                                                    C22 += A21 * B21;
                                                    C22 += A22 * B22;
                                                }

                                                C[iii * N + jjj] = C11;
                                                C[iii * N + jjj + 1] = C12;
                                                C[(iii + 1) * N + jjj] = C21;
                                                C[(iii + 1) * N + jjj + 1] = C22;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }, []{});

            bench_data << result << "   ";
        }

        {
            std::vector<double> A(N * N);
            std::vector<double> B(N * N);
            std::vector<double> C(N * N);


            auto result = run_benchmark([&]
                          {

                for (long int i = 0; i < N; ++i)
                {
                    for (long int j = 0; j < N; ++j)
                    {
                        C[i * N + j] = 0.0;
                    }
                }

                for (long int i = 0; i < N; ++i)
                {
                    for (long int j = 0; j < N; ++j)
                    {
                        for (long int k = 0; k < N; ++k)
                        {
                            C[i * N + j] += A[i * N + k] * B[j * N + k];
                        }
                    }
                }
            }, []{});

            bench_data << result << "   ";
        }

        {
            using MatrixType = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

            MatrixType A = MatrixType::Zero(N, N);
            MatrixType B = MatrixType::Zero(N, N);
            MatrixType C = MatrixType::Zero(N, N);

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
    return hpx::init(argc, argv);
}
