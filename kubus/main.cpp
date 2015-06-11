#include <hpx/config.hpp>

#include <iostream>
#include <eigen3/Eigen/Core>

#include <qbb/kubus/IR/type.hpp>

#include <qbb/kubus/runtime.hpp>

#include <qbb/kubus/tensor_variable.hpp>

#include <qbb/kubus/user_defined_plan.hpp>

#include <hpx/hpx_init.hpp>

#include <memory>
#include <cstring>
#include <utility>
#include <vector>
#include <functional>
#include <numeric>

#include <random>

// for tests
#include <chrono>

#include <qbb/kubus/allocator.hpp>

namespace qbb
{
namespace kubus
{

class mock_allocator : public allocator
{
public:
    mock_allocator(std::unique_ptr<allocator> underlying_allocator_, std::size_t allocated_memory_,
                   std::size_t total_memory_)
    : underlying_allocator_(std::move(underlying_allocator_)), allocated_memory_(allocated_memory_),
      total_memory_(total_memory_)
    {
    }

    virtual ~mock_allocator() = default;

    std::unique_ptr<memory_block> allocate(std::size_t size, std::size_t alignment)
    {
        if (allocated_memory_ + size < total_memory_)
        {
            allocated_memory_ += size;
            return underlying_allocator_->allocate(size, alignment);
        }
        else
        {
            return {};
        }
    }

    void deallocate(memory_block& mem_block)
    {
        std::size_t size = mem_block.size();

        underlying_allocator_->deallocate(mem_block);
        allocated_memory_ -= size;
    }

private:
    std::unique_ptr<allocator> underlying_allocator_;
    std::size_t allocated_memory_;
    std::size_t total_memory_;
};
}
}

using namespace qbb::kubus;

int hpx_main(int argc, char** argv)
{
    qbb::kubus::init(argc, argv);

    long int N = 2000;

    auto my_plan =
        make_plan()
            .body([](cpu_tensor_view<const double, 2> A)
                  {
                      for (long int i = 0; i < std::min((long int)(10), A.extent(0)); ++i)
                      {
                          std::cout << A(i, 0) << std::endl;
                      }
                  })
            .finalize();

    {
        qbb::kubus::index i("i");
        qbb::kubus::index j("j");
        qbb::kubus::index k("k");

        tensor<double, 2> A(N, N);
        tensor<double, 2> B(N, N);
        tensor<double, 2> C(N, N);

        tensor_expr<double, 2> zeros = def_tensor(i, j)[0];

        tensor_expr<double, 2> Cdef = def_tensor(i, j)[sum(A(i, k)*B(k, j), k)];

        A = zeros;
        B = zeros;

        C = Cdef;
        C.when_ready().wait();

        auto start = std::chrono::high_resolution_clock::now();

        //for (long int i = 0; i < 1; ++i)
        //{
        //    C = Cdef;
        //}

        C.when_ready().wait();

        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

        std::cout << duration.count() / 1.0 << " seconds" << std::endl;

        execute(my_plan, C);
        C.when_ready().wait();
    }

    /*{
        std::vector<double> A(N * N);
        std::vector<double> B(N * N);
        std::vector<double> C(N * N);

        auto start = std::chrono::high_resolution_clock::now();

        for (long int l = 0; l < 10; ++l)
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
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

        std::cout << duration.count() / 10.0 << " seconds" << std::endl;
    }*/

    /*{
        std::vector<double> A(N * N);
        std::vector<double> B(N * N);
        std::vector<double> C(N * N);

        auto start = std::chrono::high_resolution_clock::now();

        for (long int l = 0; l < 1; ++l)
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

        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

        std::cout << duration.count() / 10.0 << " seconds" << std::endl;
    }*/

    /*{
        using MatrixType = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

        MatrixType A = MatrixType::Zero(N, N);
        MatrixType B = MatrixType::Zero(N, N);
        MatrixType C = MatrixType::Zero(N, N);

        auto start = std::chrono::high_resolution_clock::now();

        for (long int l = 0; l < 1; ++l)
        {
            C.noalias() = A * B.transpose();
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

        std::cout << duration.count() / 10.0 << " seconds" << std::endl;
    }*/

    return hpx::finalize();
}

int main(int argc, char** argv)
{
    return hpx::init(argc, argv);
}