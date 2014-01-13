#ifndef QBB_SPARSE_TENSOR_HPP
#define QBB_SPARSE_TENSOR_HPP

#include <QBB/support/integers.hpp>

#include <boost/range/algorithm.hpp>

#include <vector>
#include <array>
#include <utility>
#include <functional>
#include <numeric>
#include <utility>
#include <algorithm>

namespace qbb
{

template <typename T, int Rank>
class indices_value_pair
{
public:
    indices_value_pair(std::array<index_t, Rank> indices_, T value_)
    : indices_(std::move(indices_)), value_{std::move(value_)}
    {
    }

    const std::array<index_t, Rank>& indices() const
    {
        return indices_;
    }

    T& value()
    {
        return value_;
    }

    const T& value() const
    {
        return value_;
    }

private:
    std::array<index_t, Rank> indices_;
    T value_;
};

}

namespace std
{

template <typename T, int Rank>
struct less<qbb::indices_value_pair<T, Rank>>
{
    constexpr bool operator()(const qbb::indices_value_pair<T, Rank>& lhs,
                              const qbb::indices_value_pair<T, Rank>& rhs) const
    {
        return lhs.indices() < rhs.indices();
    }
};
}

namespace qbb
{

namespace detail
{

template <typename T, int Rank>
class coordinate_list
{
public:
    using container = std::vector<indices_value_pair<T, Rank>>;
    using iterator = typename container::iterator;
    using nonzeros_range = container&;
    using const_nonzeros_range = const container&;

    template <typename Range>
    void set(Range values)
    {
        boost::sort(values, std::less<indices_value_pair<T, Rank>>());

        for (auto iter = values.begin(), end = values.end(); iter != end;)
        {
            const auto& indices = iter->indices();
            T acc = iter->value();

            ++iter;

            while (iter != end && iter->indices() == indices)
            {
                acc += iter->value();

                ++iter;
            }

            coordinates_.emplace_back(indices, acc);
        }

        assert(std::is_sorted(std::begin(coordinates_),std::end(coordinates_),std::less<indices_value_pair<T, Rank>>()));
    }

    T get(const std::array<index_t, Rank>& indices) const
    {
        auto iter = std::lower_bound(coordinates_.begin(), coordinates_.end(),
                                     indices_value_pair<T, Rank>(indices, 0),
                                     std::less<indices_value_pair<T, Rank>>());

        if (iter->indices() > indices)
        {
            return T{};
        }
        else
        {
            return iter->value();
        }
    }

    iterator begin()
    {
        return coordinates_.begin();
    }

    iterator end()
    {
        return coordinates_.end();
    }

    nonzeros_range nonzeros()
    {
        return coordinates_;
    }

    const_nonzeros_range nonzeros() const
    {
        return coordinates_;
    }
    
    index_t nonzero_count() const
    {
        return to_uindex(coordinates_.size());
    }

private:
    container coordinates_;
};
}

template <typename T, int Rank>
class sparse_tensor
{
public:
    using iterator = typename detail::coordinate_list<T, Rank>::iterator;
    using nonzeros_range = typename detail::coordinate_list<T, Rank>::nonzeros_range;
    using const_nonzeros_range = typename detail::coordinate_list<T, Rank>::const_nonzeros_range;

    sparse_tensor() = default;
    
    template <typename... Integers>
    explicit sparse_tensor(Integers... shape_)
    : shape_({to_uindex(shape_)...})
    {
        static_assert(Rank == sizeof...(shape_),
                      "sparse_tensor: insufficient number of dimensions");
    }

    template <typename Range>
    void set(Range values)
    {
        data_.set(std::move(values));
    }

    template <typename... Indices>
    T operator()(Indices... indices) const
    {
        static_assert(Rank == sizeof...(indices), "sparse_tensor: insufficient number of indices");

        return data_.get({to_uindex(indices)...});
    }

    T operator()(const std::array<index_t, Rank>& indices) const
    {
        assert(indices.size() == Rank);

        return data_.get(indices);
    }

    const std::vector<index_t>& shape()
    {
        return shape_;
    }

    iterator begin()
    {
        return data_.begin();
    }

    iterator end()
    {
        return data_.end();
    }

    nonzeros_range nonzeros()
    {
        return data_.nonzeros();
    }

    const_nonzeros_range nonzeros() const
    {
        return data_.nonzeros();
    }
    
    const std::vector<index_t>& shape() const
    {
        return shape_;
    }
    
    double sparsity()
    {
        return double(data_.nonzero_count()) / double(number_of_elements());
    }

    std::size_t memory_footprint() const
    {
        return data_.nonzero_count() * sizeof(T);
    }

private:
    index_t number_of_elements() const
    {
        return std::accumulate(shape_.begin(), shape_.end(), index_t{1},
                               std::multiplies<index_t>{});
    }

    detail::coordinate_list<T, Rank> data_;
    std::vector<index_t> shape_;
};

}

#endif