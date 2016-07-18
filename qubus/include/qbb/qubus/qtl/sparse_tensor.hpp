#ifndef QBB_QUBUS_SPARSE_TENSOR_HPP_H
#define QBB_QUBUS_SPARSE_TENSOR_HPP_H

#include <hpx/config.hpp>

#include <qbb/qubus/get_view.hpp>
#include <qbb/qubus/host_object_views.hpp>

#include <qbb/qubus/qtl/indexed_tensor_expr_context.hpp>

#include <qbb/qubus/object_factory.hpp>
#include <qbb/qubus/qtl/assembly_tensor.hpp>

#include <qbb/qubus/host_object_views.hpp>
#include <qbb/qubus/object_materializer.hpp>

#include <qbb/util/assert.hpp>
#include <qbb/util/index_space.hpp>
#include <qbb/util/integers.hpp>
#include <qbb/util/make_array.hpp>

#include <array>
#include <map>

namespace qbb
{
namespace qubus
{
namespace qtl
{

template <typename T, long int Rank>
class sparse_tensor_info
{
public:
    sparse_tensor_info() = default;

    explicit sparse_tensor_info(object data_) : data_(std::move(data_))
    {
    }

    object get_object() const
    {
        return data_;
    }

private:
    object data_;
};

template <typename T, long int Rank>
class sparse_tensor
    : public tensor_expr_<typename boost::proto::terminal<sparse_tensor_info<T, Rank>>::type>
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

            const auto& shape = data.shape();

            auto shape_ = std::vector<util::index_t>(shape.begin(), shape.end());

            auto outer_shape =
                util::make_array<util::index_t, Rank - 2>(shape.begin(), shape.end() - 2);

            util::index_t innermost_dense_extent = shape[shape.size() - 2];

            util::index_t padded_nnz = 0;

            auto outer_space = util::make_index_space_from_shape(outer_shape);

            for (const auto& offset : outer_space)
            {
                for (util::index_t i = 0; i < innermost_dense_extent; i += block_width)
                {
                    util::index_t max_depth = 0;

                    for (util::index_t ii = i;
                         ii < std::min(i + block_width, innermost_dense_extent); ++ii)
                    {
                        max_depth = std::max(
                            max_depth,
                            util::to_uindex(
                                data.root()
                                    .data()(product(offset, util::offset<util::index_t, 1>(ii)))
                                    .size()));
                    }

                    padded_nnz += max_depth * block_width;
                }
            }

            util::index_t num_chuncks =
                outer_space.cardinality() * innermost_dense_extent / block_width;

            if ((outer_space.cardinality() * innermost_dense_extent) % block_width != 0)
            {
                num_chuncks += 1;
            }

            sparse_tensor_layout layout(num_chuncks, padded_nnz);

            auto sparse_tensor_object = get_runtime().get_object_factory().create_sparse_tensor(
                types::double_(), shape_, layout);

            auto sparse_tensor_view =
                get_view<mutable_cpu_sparse_tensor_view<T, Rank>>(sparse_tensor_object).get();

            util::index_t cs = 0;

            for (const auto& offset : outer_space)
            {
                util::index_t linearized_offset = linearize(offset, outer_space);

                for (util::index_t i = 0; i < innermost_dense_extent; i += block_width)
                {
                    util::index_t max_depth = 0;

                    for (util::index_t ii = i;
                         ii < std::min(i + block_width, innermost_dense_extent); ++ii)
                    {
                        max_depth = std::max(
                            max_depth,
                            util::to_uindex(
                                data.root()
                                    .data()(product(offset, util::offset<util::index_t, 1>(ii)))
                                    .size()));
                    }

                    sparse_tensor_view.cl()((linearized_offset * innermost_dense_extent + i) /
                                            block_width) = max_depth;
                    sparse_tensor_view.cs()((linearized_offset * innermost_dense_extent + i) /
                                            block_width) = cs;

                    for (util::index_t ii = i;
                         ii < std::min(i + block_width, innermost_dense_extent); ++ii)
                    {
                        util::index_t j = 0;
                        for (const auto& index_value : data.root().data()(
                                 product(offset, util::offset<util::index_t, 1>(ii))))
                        {
                            sparse_tensor_view.values()(cs + block_width * j + (ii - i)) =
                                index_value.second;
                            sparse_tensor_view.col()(cs + block_width * j + (ii - i)) =
                                index_value.first;
                            j++;
                        }

                        for (; j < max_depth; ++j)
                        {
                            sparse_tensor_view.values()(cs + block_width * j + (ii - i)) = 0;
                            sparse_tensor_view.col()(cs + block_width * j + (ii - i)) = 0;
                        }
                    }

                    cs += block_width * max_depth;
                }
            }

            for (util::index_t i = 0; i < Rank; ++i)
            {
                sparse_tensor_view.shape()(i) = shape_[i];
            }

            boost::proto::value(*this) = sparse_tensor_info<T, Rank>(sparse_tensor_object);
        }
    }

    object get_object() const
    {
        return boost::proto::value(*this).get_object();
    }
};
}
}
}

#endif
