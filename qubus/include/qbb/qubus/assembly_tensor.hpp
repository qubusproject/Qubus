#ifndef QBB_QUBUS_ASSEMBLY_TENSOR_HPP
#define QBB_QUBUS_ASSEMBLY_TENSOR_HPP

#include <qbb/util/integers.hpp>

#include <qbb/util/hash.hpp>
#include <qbb/util/multi_array.hpp>

#include <array>
#include <unordered_map>

namespace qbb
{
namespace qubus
{

namespace detail
{

template<typename T, long int Rank>
struct array_hasher
{
    std::size_t operator()(const std::array<T, Rank>& value) const noexcept
    {
        std::size_t seed = 0;

        for (long int i = 0; i < Rank; ++i)
        {
            util::hash_combine(seed, value[i]);
        }

        return seed;
    }
};

template<typename T, typename IndexType, long int Rank>
class assembly_index_leaf
{
public:
    using index_type = IndexType;

    assembly_index_leaf(std::array<util::index_t, Rank> offset_, std::array<util::index_t, Rank> shape_)
    {
    }

    void add_nonzero(std::array<IndexType, Rank> indices, T value)
    {
        table_[indices] += value;
    }

private:

    std::unordered_map<std::array<IndexType, Rank>, T, array_hasher<IndexType, Rank>> table_;
};

template<typename T, typename ParentIndexType, long int Rank, typename ChildType>
class assembly_index_node
{
public:
    assembly_index_node(std::array<ParentIndexType, Rank> offset_, std::array<ParentIndexType, Rank> shape_)
    : offset_(offset_), shape_(shape_)
    {
    }

    void add_nonzero(std::array<util::index_t, Rank> indices, T value)
    {
        std::array<typename ChildType::index_type, Rank> local_indices;
        std::array<util::index_t, Rank> tile_indices;

        for (long int i = 0; i < Rank; ++i)
        {
            tile_indices[i] = indices[i]/256;

            local_indices[i] = indices[i] - tile_indices[i]*256;
        }

        auto iter = table_.find(tile_indices);

        if (iter == table_.end())
        {
            std::array<util::index_t, Rank> offset;
            std::array<util::index_t, Rank> shape;

            for (long int i = 0; i < Rank; ++i)
            {
                offset[i] = tile_indices[i] * 256;
                shape[i] = 256;
            }

            iter = table_.emplace(tile_indices, ChildType(offset, shape)).first;
        }

        iter->second.add_nonzero(local_indices, value);
    }

private:
    std::array<ParentIndexType, Rank> offset_;
    std::array<ParentIndexType, Rank> shape_;
    std::unordered_map<std::array<util::index_t, Rank>, ChildType, array_hasher<util::index_t, Rank>> table_;
};

template<typename T, long int Rank>
using root_type = assembly_index_node<T, long int, Rank, assembly_index_leaf<T, char, Rank>>;

template<typename T, long int Rank>
class assembly_index
{
public:
    explicit assembly_index(std::array<util::index_t, Rank> shape_)
    : root_({{0, 0}}, shape_)
    {
    }

    void add_nonzero(std::array<util::index_t, Rank> indices, T value)
    {
        root_.add_nonzero(indices, value);
    }

private:
    root_type<T, Rank> root_;
};

}

template<typename T, long int Rank>
class assembly_tensor
{
public:
    assembly_tensor(util::index_t N, util::index_t M)
    : index_({N, M})
    {
    }

    void add_nonzero(std::array<util::index_t, Rank> indices, T value)
    {
        index_.add_nonzero(indices, value);
    }
private:
    detail::assembly_index<T, Rank> index_;
};

}
}

#endif
