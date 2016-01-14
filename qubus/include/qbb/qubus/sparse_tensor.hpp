#ifndef QBB_QUBUS_SPARSE_TENSOR_HPP_H
#define QBB_QUBUS_SPARSE_TENSOR_HPP_H

#include <hpx/config.hpp>

#include <qbb/qubus/indexed_tensor_expr_context.hpp>

#include <qbb/qubus/object_factory.hpp>
#include <qbb/qubus/assembly_tensor.hpp>

#include <qbb/qubus/local_runtime.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/index_space.hpp>
#include <qbb/util/assert.hpp>
#include <qbb/util/make_array.hpp>

#include <map>
#include <array>

namespace qbb
{
namespace qubus
{

template <typename T, util::index_t Rank>
class cpu_array_view
{
public:
    using value_type = T;

    template <typename... Indices>
    T& operator()(Indices... indices)
    {
        static_assert(Rank == sizeof...(indices), "insufficient number of indices");

        util::index_t linear_index = 0;

        std::array<util::index_t, Rank> indices_{{util::to_uindex(indices)...}};

        for (util::index_t i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices_[i];
        }

        QBB_ASSERT(linear_index >= 0, "Linear index can't be negative.");

        return data_[linear_index];
    }

    template <typename... Indices>
    const T& operator()(Indices... indices) const
    {
        static_assert(Rank == sizeof...(indices), "insufficient number of indices");

        util::index_t linear_index = 0;

        std::array<util::index_t, Rank> indices_{util::to_uindex(indices)...};

        for (util::index_t i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices_[i];
        }

        QBB_ASSERT(linear_index >= 0, "Linear index can't be negative.");

        return data_[linear_index];
    }

    util::index_t rank() const
    {
        return rank_;
    }

    util::index_t extent(util::index_t dim) const
    {
        return shape_[dim];
    }

    // TODO: implement shape

    static cpu_array_view<T, Rank> construct(void* metadata)
    {
        auto array_md = static_cast<array_metadata*>(metadata);

        return cpu_array_view<T, Rank>(Rank, static_cast<util::index_t*>(array_md->shape),
                                       static_cast<T*>(array_md->data));
    }

private:
    cpu_array_view(util::index_t rank_, util::index_t* shape_, T* data_)
    : rank_(rank_), shape_(shape_), data_(data_)
    {
    }

    util::index_t rank_;
    util::index_t* shape_;
    T* data_;
};

template <typename T, long int Rank>
class mutable_cpu_sparse_tensor_view
{
public:
    using value_type = T;

    static mutable_cpu_sparse_tensor_view<T, Rank> construct(void* metadata)
    {
        auto struct_md = static_cast<void**>(metadata);

        auto sell_md = static_cast<void**>(struct_md[0]);

        auto values = cpu_array_view<T, 1>::construct(sell_md[0]);
        auto col = cpu_array_view<util::index_t, 1>::construct(sell_md[1]);
        auto cs = cpu_array_view<util::index_t, 1>::construct(sell_md[2]);
        auto cl = cpu_array_view<util::index_t, 1>::construct(sell_md[3]);

        auto shape_md = static_cast<void*>(struct_md[1]);

        auto shape = cpu_array_view<util::index_t, 1>::construct(shape_md);

        return mutable_cpu_sparse_tensor_view<T, Rank>(col, cl, cs, values, shape);
    }

    void dump()
    {
        //FIXME: This does not work for arbitrary tensors.
        std::cout << "sparse tensor\n" << std::endl;

        std::cout << col_.extent(0) << std::endl;
        std::cout << cl_.extent(0) << std::endl;
        std::cout << cs_.extent(0) << std::endl;
        std::cout << values_.extent(0) << std::endl;

        std::cout << std::endl;

        for (util::index_t i = 0; i < cl_.extent(0); ++i)
        {
            std::cout << cl_(i) << "  ";
        }
        std::cout << std::endl;

        for (util::index_t i = 0; i < cs_.extent(0); ++i)
        {
            std::cout << cs_(i) << "  ";
        }
        std::cout << std::endl;

        std::cout << std::endl;

        util::index_t N = 10;

        for (util::index_t i = 0; i < (N / 4)*4; i += 4)
        {
            for (util::index_t k = 0; k < 4; ++k)
            {
                for (util::index_t j = 0; j < cl()(i/4); ++j)
                {
                    std::cout << values()(cs()(i/4) + 4 * j + k) << " ";
                }
                std::cout << std::endl;
            }

            std::cout << "-----------" << std::endl;
        }

        for (util::index_t i = (N / 4)*4; i < N; i += 4)
        {
            for (util::index_t k = 0; k < N % 4; ++k)
            {
                for (util::index_t j = 0; j < cl()(i/4); ++j)
                {
                    std::cout << values()(cs()(i/4) + 4 * j + k) << " ";
                }
                std::cout << std::endl;
            }

            std::cout << "-----------" << std::endl;
        }
    }

    cpu_array_view<util::index_t, 1> col()
    {
        return col_;
    }

    cpu_array_view<util::index_t, 1> cl()
    {
        return cl_;
    }

    cpu_array_view<util::index_t, 1> cs()
    {
        return cs_;
    }

    cpu_array_view<T, 1> values()
    {
        return values_;
    }

    cpu_array_view<util::index_t, 1> shape()
    {
        return shape_;
    };
private:
    mutable_cpu_sparse_tensor_view(cpu_array_view<util::index_t, 1> col_,
                                   cpu_array_view<util::index_t, 1> cl_,
                                   cpu_array_view<util::index_t, 1> cs_,
                                   cpu_array_view<T, 1> values_,
                                   cpu_array_view<util::index_t, 1> shape_)
    : col_(std::move(col_)), cl_(std::move(cl_)), cs_(std::move(cs_)), values_(std::move(values_)), shape_(std::move(shape_))
    {
    }

    cpu_array_view<util::index_t, 1> col_;
    cpu_array_view<util::index_t, 1> cl_;
    cpu_array_view<util::index_t, 1> cs_;
    cpu_array_view<T, 1> values_;
    cpu_array_view<util::index_t, 1> shape_;
};

template <typename T, long int Rank>
class sparse_tensor : public tensor_expr_<typename boost::proto::terminal< std::shared_ptr<struct_>>::type>
{
public:
    sparse_tensor(assembly_tensor<T, Rank>&& data)
    {
        if (Rank == 1)
        {
            // TODO: Implement a sparse vector format.
            throw 0;
        }
        else
        {
            util::index_t block_width = 4;

            const auto &shape = data.shape();

            auto shape_ = std::vector<util::index_t>(shape.begin(), shape.end());

            auto outer_shape = util::make_array<util::index_t, Rank - 2>(shape.begin(), shape.end() - 2);

            util::index_t innermost_dense_extent = shape[shape.size() - 2];

            util::index_t padded_nnz = 0;

            auto outer_space = util::make_index_space_from_shape(outer_shape);

            for (const auto &offset : outer_space)
            {
                for (util::index_t i = 0; i < innermost_dense_extent; i += block_width)
                {
                    util::index_t max_depth = 0;

                    for (util::index_t ii = i; ii < std::min(i + block_width, innermost_dense_extent); ++ii)
                    {
                        max_depth = std::max(max_depth, util::to_uindex(
                                data.root().data()(product(offset, util::offset<util::index_t, 1>(ii))).size()));
                    }

                    padded_nnz += max_depth * block_width;
                }
            }

            util::index_t num_chuncks = outer_space.cardinality() * innermost_dense_extent / block_width;

            if ((outer_space.cardinality() * innermost_dense_extent) % block_width != 0)
            {
                num_chuncks += 1;
            }

            sparse_tensor_layout layout(num_chuncks, padded_nnz);

            boost::proto::value(*this) = get_runtime().get_object_factory().create_sparse_tensor(types::double_(), shape_,
                                                                              layout);


            const auto setup =
                    make_computelet()
                            .body([&](mutable_cpu_sparse_tensor_view<T, Rank> s)
                                  {
                                      util::index_t cs = 0;

                                      for (const auto &offset : outer_space)
                                      {
                                          util::index_t linearized_offset = linearize(offset, outer_space);

                                          for (util::index_t i = 0; i < innermost_dense_extent; i += block_width)
                                          {
                                              util::index_t max_depth = 0;

                                              for (util::index_t ii = i;
                                                   ii < std::min(i + block_width, innermost_dense_extent); ++ii)
                                              {
                                                  max_depth = std::max(
                                                          max_depth, util::to_uindex(data.root().data()(
                                                          product(offset, util::offset<util::index_t, 1>(ii))).size()));
                                              }

                                              s.cl()((linearized_offset * innermost_dense_extent + i) /
                                                     block_width) = max_depth;
                                              s.cs()((linearized_offset * innermost_dense_extent + i) /
                                                     block_width) = cs;

                                              for (util::index_t ii = i; ii < std::min(i + block_width, innermost_dense_extent); ++ii)
                                              {
                                                  util::index_t j = 0;
                                                  for (const auto &index_value : data.root().data()(
                                                          product(offset, util::offset<util::index_t, 1>(ii))))
                                                  {
                                                      s.values()(cs + block_width * j + (ii - i)) = index_value.second;
                                                      s.col()(cs + block_width * j + (ii - i)) = index_value.first;
                                                      j++;
                                                  }

                                                  for (; j < max_depth; ++j)
                                                  {
                                                      s.values()(cs + block_width * j + (ii - i)) = 0;
                                                      s.col()(cs + block_width * j + (ii - i)) = 0;
                                                  }
                                              }

                                              cs += block_width * max_depth;
                                          }
                                      }

                                      for (util::index_t i = 0; i < Rank; ++i)
                                      {
                                          s.shape()(i) = shape_[i];
                                      }
                                  })
                            .finalize();

            execute(setup, *this);

            get_runtime().when_ready(tensor()).wait();
        }
    }

    std::shared_ptr<struct_> get_object() const
    {
        return boost::proto::value(*this);
    }

private:
    const struct_& tensor() const
    {
        return *boost::proto::value(*this);
    }

    struct_& tensor()
    {
        return *boost::proto::value(*this);
    }
};
}
}

#endif
