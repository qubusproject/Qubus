#ifndef QUBUS_SPARSE_TENSOR_HPP_H
#define QUBUS_SPARSE_TENSOR_HPP_H

#include <hpx/config.hpp>

#include <qubus/get_view.hpp>
#include <qubus/host_object_views.hpp>

#include <qubus/qtl/assembly_tensor.hpp>
#include <qubus/qtl/ast.hpp>

#include <qubus/host_object_views.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/index_space.hpp>
#include <qubus/util/integers.hpp>
#include <qubus/util/make_array.hpp>

#include <array>
#include <map>

namespace qubus
{
namespace qtl
{
namespace ast
{
template <typename T, long int Rank>
class sparse_tensor
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

            data_ = std::move(sparse_tensor_object);
        }
    }

    template <typename... Indices>
    auto operator()(Indices... indices) const
    {
        static_assert(are_all_indices<Indices...>(), "Expecting indices.");

        return subscripted_tensor<sparse_tensor<T, Rank>, Indices...>(*this, indices...);
    }

    object get_object() const
    {
        return data_;
    }

private:
    object data_;
};
}

template <typename T, long int Rank>
using sparse_tensor = ast::sparse_tensor<T, Rank>;
}

template <typename T, long int Rank>
struct associated_qubus_type<qtl::ast::sparse_tensor<T, Rank>>
{
    static type get()
    {
        return types::sparse_tensor(associated_qubus_type<T>::get());
    }
};
}

#endif
