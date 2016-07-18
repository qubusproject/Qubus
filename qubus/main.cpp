#include <hpx/config.hpp>

#include <iostream>
//#include <eigen3/Eigen/Core>

#include <qbb/qubus/qubus.hpp>

#include <qbb/qubus/qtl/all.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/parallel/algorithms/for_loop.hpp>

#include <memory>
#include <cstring>
#include <utility>
#include <vector>
#include <functional>
#include <numeric>

#include <random>

// for tests
#include <chrono>

#include <qbb/qubus/allocator.hpp>

namespace qbb
{
namespace qubus
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

using namespace qbb::qubus;

void print_vector(host_tensor_view<const double, 1> v)
{
    for (long int i = 0; i < 10; ++i)
    {
        std::cout << v(i) << std::endl;
    }
}

int hpx_main(int argc, char** argv)
{
    hpx::cout << "Init" << hpx::endl;

    qbb::qubus::init(argc, argv);

    using namespace qbb::qubus::qtl;

    hpx::cout << "------------------------------------------------------" << hpx::endl;

    auto num_localities = hpx::get_num_localities_sync();

    long int N = 3000;

    std::vector<tensor<double, 2>> matrices;
    std::vector<tensor<double, 1>> vectors;
    std::vector<tensor<double, 1>> results;

    for (long int i = 0; i < num_localities; ++i)
    {
        matrices.emplace_back(N, N);
    }

    for (long int i = 0; i < num_localities; ++i)
    {
        vectors.emplace_back(N);
    }

    for (long int i = 0; i < num_localities; ++i)
    {
        results.emplace_back(N);
    }

    qtl::index i, j;

    std::vector<tensor_expr<double, 1>> codes;

    for (long int loc = 0; loc < num_localities; ++loc)
    {
        auto A = matrices[loc];
        auto b = vectors[loc];

        tensor_expr<double, 1> code = def_tensor(i)[sum(A(i, j) * b(j), j)];
        codes.push_back(code);
    }

    for (long int loc = 0; loc < num_localities; ++loc)
    {
        results[loc] = codes[loc];
    }

    for (long int loc = 0; loc < num_localities; ++loc)
    {
        results[loc].when_ready().wait();
    }

    hpx::cout << "------------------------------------------------------" << hpx::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (long int k = 0; k < 1; ++k)
    {
        hpx::cout << "iteration " << k << hpx::endl;

        std::vector<hpx::future<void>> futures;
        futures.reserve(num_localities);

        for (long int loc = 0; loc < num_localities; ++loc)
        {
            futures.push_back(hpx::async([&results, &codes, loc]
                       {
                           results[loc] = codes[loc];
                       }));
        }

        hpx::cout << "end iteration " << k << hpx::endl;

        hpx::wait_all(std::move(futures));
    }

    hpx::cout << "terminating!!!" << hpx::endl;

    for (long int loc = 0; loc < num_localities; ++loc)
    {
        results[loc].when_ready().wait();
        hpx::cout << "calculation " << loc << " terminated!!!" << hpx::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

    hpx::cout << duration.count() << " seconds" << hpx::endl;

    hpx::cout << "finished!!!" << hpx::endl;

    /*long int N = 10;
     *
    tensor<double, 1> b(N);
    tensor<double, 1> rb(N);

    qbb::qubus::index i;

    tensor_expr<double, 1> one = def_tensor (i) [1];

    b = one;

    auto view = get_view<host_tensor_view<double, 1>>(b).get();

    for (long int i = 0; i < 10; ++i)
    {
        std::cout << view(i) << std::endl;
    }

    std::vector<object_client> components;
    type obj_type = types::double_();
    object_client obj = hpx::new_<aggregate_object>(hpx::find_here(), obj_type, components);

    auto aggregate = object_cast<aggregate_object_client>(obj);*/

    /*long int N = 1000;

    long int samples = 10;

    auto my_plan =
        make_computelet()
            .body([](cpu_tensor_view<const double, 2> A)
                  {
                      for (long int i = 0; i < std::min((long int)(10), A.extent(0)); ++i)
                      {
                          std::cout << A(i, 0) << std::endl;
                      }
                  })
            .finalize();

    {
        qbb::qubus::index i("i");
        qbb::qubus::index j("j");
        qbb::qubus::index k("k");

        tensor<double, 2> A(N, N);
        tensor<double, 2> B(N, N);
        tensor<double, 2> C(N, N);

        tensor_expr<double, 2> zeros = def_tensor(i, j)[0];

        tensor_expr<double, 2> Cdef = def_tensor(i, j)[sum(A(i, k)*B(k, j), k)];

        A = zeros;
        B = zeros;

        C = Cdef;
        C.when_ready().wait();

        long int M = 10000000;

        std::vector<qbb::util::index_t> indices(2*M);

        std::random_device rd;

        std::mt19937 engine(rd());

        std::uniform_int_distribution<qbb::util::index_t> dist(0, N);

        for (long int i = 0; i < M; ++i)
        {
            indices[2*i] = dist(engine);
            indices[2*i] = dist(engine);
        }

        auto start = std::chrono::high_resolution_clock::now();

        for (long int i = 0; i < samples; ++i)
        {
            C = Cdef;
        }

        C.when_ready().wait();

        assembly_tensor<double, 2> D(N, N);

        for (long int i = 0; i < M; ++i)
        {
            D.add_nonzero({{indices[2*i], indices[2*i + 1]}}, 1.0);
        }


        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

        std::cout << duration.count() / static_cast<double>(samples) << " seconds" << std::endl;

        execute(my_plan, C);
        C.when_ready().wait();
    }*/

    /*{
        std::vector<double> A(N * N);
        std::vector<double> B(N * N);
        std::vector<double> C(N * N);

        auto start = std::chrono::high_resolution_clock::now();

        for (long int l = 0; l < samples; ++l)
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

        std::cout << duration.count() / static_cast<double>(samples) << " seconds" << std::endl;
    }*/

    /*{
        std::vector<double> A(N * N);
        std::vector<double> B(N * N);
        std::vector<double> C(N * N);

        auto start = std::chrono::high_resolution_clock::now();

        for (long int l = 0; l < samples; ++l)
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
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

        std::cout << duration.count() / static_cast<double>(samples) << " seconds" << std::endl;
    }*/

    /*{
        using MatrixType = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

        MatrixType A = MatrixType::Zero(N, N);
        MatrixType B = MatrixType::Zero(N, N);
        MatrixType C = MatrixType::Zero(N, N);

        auto start = std::chrono::high_resolution_clock::now();

        for (long int l = 0; l < samples; ++l)
        {
            C.noalias() = A * B;
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

        std::cout << duration.count() / static_cast<double>(samples) << " seconds" << std::endl;
    }*/

    return hpx::finalize();
}

int main(int argc, char** argv)
{
    return hpx::init(argc, argv);
}