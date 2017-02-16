#ifndef QBB_QUBUS_ASSEMBLY_TENSOR_HPP
#define QBB_QUBUS_ASSEMBLY_TENSOR_HPP

#include <qbb/util/integers.hpp>

#include <qbb/util/hash.hpp>
#include <qbb/util/multi_array.hpp>

#include <array>
#include <map>
#include <unordered_map>
#include <limits>

namespace qubus
{
namespace qtl
{

namespace detail
{
template <typename T, long int Rank>
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

template<typename T, long unsigned int Rank>
std::array<T, Rank - 1> discard_last_dimension(const std::array<T, Rank>& indices)
{
    std::array<T, Rank - 1> leading_indices;

    for (std::size_t i = 0; i < Rank - 1; ++i)
    {
        leading_indices[i] = indices[i];
    }

    return leading_indices;
}

template <typename T, typename IndexType, long int Rank>
class assembly_index_leaf
{
public:
    using index_type = IndexType;
    using value_type = T;
    static constexpr long int rank = Rank;
    static constexpr IndexType extent = std::numeric_limits<IndexType>::max();

    explicit assembly_index_leaf(std::array<util::index_t, Rank> shape_)
    : table_(detail::discard_last_dimension(shape_))
    {
    }

    void add_nonzero(std::array<index_type, rank> indices, T value)
    {
        // TODO: Use an array_view to slice the last index away.
        std::array<index_type, rank - 1> leading_indices = detail::discard_last_dimension(indices);

        table_(leading_indices)[indices.back()] += value;
    }

    const util::multi_array<std::map<index_type, T>, rank - 1>& data() const
    {
        return table_;
    };
private:
    util::multi_array<std::map<index_type, T>, rank - 1> table_;
};

template <typename IndexType, typename ChildType>
class assembly_index_node
{
public:
    using index_type = IndexType;
    using local_index_type = typename ChildType::index_type;
    using value_type = typename ChildType::value_type;
    static constexpr long int rank = ChildType::rank;
    static constexpr local_index_type extent = std::numeric_limits<local_index_type>::max();

    void add_nonzero(std::array<index_type, rank> indices, value_type value)
    {
        std::array<index_type, rank> tile_indices;
        std::array<local_index_type, rank> local_indices;

        for (long int i = 0; i < rank; ++i)
        {
            tile_indices[i] = indices[i] / ChildType::extent;
            local_indices[i] = indices[i] - tile_indices[i] * ChildType::extent;
        }

        auto iter = table_.find(tile_indices);

        if (iter == table_.end())
        {
            iter = table_.emplace(tile_indices, ChildType()).first;
        }

        iter->second.add_nonzero(local_indices, value);
    }

private:
    std::unordered_map<std::array<index_type, rank>, ChildType, array_hasher<index_type, rank>>
        table_;
};

template <typename T, long int Rank>
using root_type = assembly_index_leaf<T, util::index_t, Rank>;

template <typename T, long int Rank>
class assembly_index
{
public:
    explicit assembly_index(std::array<util::index_t, Rank> shape_)
    : root_(std::move(shape_))
    {
    }

    void add_nonzero(std::array<util::index_t, Rank> indices, T value)
    {
        root_.add_nonzero(indices, value);
    }

    const root_type<T, Rank>& root() const
    {
        return root_;
    }
private:
    root_type<T, Rank> root_;
};
}

template <typename T, long int Rank>
class assembly_tensor
{
public:
    assembly_tensor(util::index_t N, util::index_t M)
    : index_({N, M}), shape_{N, M}
    {
    }

    explicit assembly_tensor(std::array<util::index_t, Rank> shape_)
    : index_(shape_), shape_(shape_)
    {
    }

    void add_nonzero(std::array<util::index_t, Rank> indices, T value)
    {
        index_.add_nonzero(indices, value);
    }

    const std::array<util::index_t, Rank>& shape() const
    {
        return shape_;
    }

    const detail::root_type<T, Rank>& root() const
    {
        return index_.root();
    }
private:
    detail::assembly_index<T, Rank> index_;
    std::array<util::index_t, Rank> shape_;
};
}
}

#endif
