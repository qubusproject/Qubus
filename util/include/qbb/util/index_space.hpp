#ifndef QBB_UTIL_INDEX_SPACE_HPP
#define QBB_UTIL_INDEX_SPACE_HPP

#include <qbb/util/concat.hpp>
#include <qbb/util/assert.hpp>
#include <qbb/util/unused.hpp>

#include <array>
#include <vector>
#include <utility>
#include <iterator>
#include <memory>
#include <type_traits>
#include <numeric>

namespace qubus
{
namespace util
{

constexpr long int dynamic_rank = -1;

template <typename IndexType, long int Rank = dynamic_rank>
class offset
{
public:
    offset() = default;

    IndexType& operator[](long int dim)
    {
        return components_[dim];
    }

    const IndexType& operator[](long int dim) const
    {
        return components_[dim];
    }

    friend bool operator==(const offset<IndexType, Rank>& lhs, const offset<IndexType, Rank>& rhs)
    {
        return lhs.components_ == rhs.components_;
    }

private:
    std::array<IndexType, Rank> components_;
};

template <typename IndexType>
class offset<IndexType, 1>
{
public:
    offset() = default;

    offset(IndexType value_) : value_(value_)
    {
    }

    IndexType& operator[](long int QBB_UNUSED(dim))
    {
        return value_;
    }

    const IndexType& operator[](long int QBB_UNUSED(dim)) const
    {
        return value_;
    }

    friend bool operator==(const offset<IndexType, 1>& lhs, const offset<IndexType, 1>& rhs)
    {
        return lhs.value_ == rhs.value_;
    }

private:
    IndexType value_;
};

template <typename IndexType>
class offset<IndexType, dynamic_rank>
{
public:
    offset() = default;

    IndexType& operator[](long int dim)
    {
        return components_[dim];
    }

    friend bool operator==(const offset<IndexType>& lhs, const offset<IndexType>& rhs)
    {
        return lhs.components_ == rhs.components_;
    }

private:
    std::vector<IndexType> components_;
};

template <typename IndexType, long int Rank>
bool operator!=(const offset<IndexType, Rank>& lhs, const offset<IndexType, Rank>& rhs)
{
    return !(lhs == rhs);
}

template <typename IndexType, long int LHSRank, long int RHSRank>
offset<IndexType, LHSRank + RHSRank> product(const offset<IndexType, LHSRank>& lhs,
                                             const offset<IndexType, RHSRank>& rhs)
{
    offset<IndexType, LHSRank + RHSRank> result;

    for (long int i = 0; i < LHSRank; ++i)
    {
        result[i] = lhs[i];
    }

    for (long int i = 0; i < RHSRank; ++i)
    {
        result[LHSRank + i] = rhs[i];
    }

    return result;
}

template <typename IndexType, long int Rank = dynamic_rank>
class index_space
{
public:
    using offset_type = offset<IndexType, Rank>;

    explicit index_space(std::array<std::array<IndexType, 3>, Rank> bounds_) : bounds_(bounds_)
    {
    }

    constexpr long int rank() const
    {
        return Rank;
    }

    IndexType cardinality() const
    {
        return std::accumulate(bounds_.begin(), bounds_.end(), 1,
                               [](const IndexType& acc, const std::array<IndexType, 3>& value)
                               {
                                   return acc * (value[1] - value[0]) / value[2];
                               });
    }

    IndexType lower_bound(long int dim) const
    {
        return bounds_[dim][0];
    }

    IndexType upper_bound(long int dim) const
    {
        return bounds_[dim][1];
    }

    IndexType stride(long int dim) const
    {
        return bounds_[dim][2];
    }

    IndexType extent(long int dim) const
    {
        return upper_bound(dim) - lower_bound(dim);
    }

    index_space<IndexType, 1> operator()(long int dim) const
    {
        return index_space<IndexType, 1>({{bounds_[dim]}});
    }

    template <long int LHSRank, long int RHSRank>
    friend index_space<IndexType, LHSRank + RHSRank>
    product(const index_space<IndexType, LHSRank>& lhs, const index_space<IndexType, RHSRank>& rhs)
    {
        return index_space<IndexType, LHSRank + RHSRank>(concat(lhs.bounds_, rhs.bounds_));
    }

private:
    std::array<std::array<IndexType, 3>, Rank> bounds_;
};

template <typename IndexType>
class index_space<IndexType, dynamic_rank>
{
public:
    using offset_type = offset<IndexType>;

    explicit index_space(std::vector<std::array<IndexType, 3>> bounds_) : bounds_(bounds_)
    {
    }

    template <long int Rank>
    index_space(const index_space<IndexType, Rank>& other)
    {
        bounds_.reserve(other.rank());

        for (long int i = 0; i < other.rank(); ++i)
        {
            bounds_.emplace_back({other.lower_bound(i), other.upper_bound(i), other.stride(i)});
        }
    }

    long int rank() const
    {
        // TODO: add check

        return bounds_.size();
    }

    IndexType cardinality() const
    {
        return std::accumulate(bounds_.begin(), bounds_.end(), 1,
                               [](const IndexType& acc, const std::array<IndexType, 3>& value)
                               {
                                   return acc * (value[1] - value[0]) / value[2];
                               });
    }

    IndexType lower_bound(long int dim) const
    {
        return bounds_[dim][0];
    }

    IndexType upper_bound(long int dim) const
    {
        return bounds_[dim][1];
    }

    IndexType stride(long int dim) const
    {
        return bounds_[dim][2];
    }

    index_space<IndexType, 1> operator()(long int dim) const
    {
        return index_space<IndexType, 1>({{bounds_[dim]}});
    }

    friend index_space<IndexType> product(const index_space<IndexType>& lhs,
                                          const index_space<IndexType>& rhs)
    {
        std::vector<std::array<IndexType, 3>> new_bounds;

        for (const auto& value : lhs.bounds_)
        {
            new_bounds.push_back(value);
        }

        for (const auto& value : rhs.bounds_)
        {
            new_bounds.push_back(value);
        }

        return index_space<IndexType>(std::move(new_bounds));
    }

private:
    std::vector<std::array<IndexType, 3>> bounds_;
};

template <typename IndexType, long int Rank>
class index_space_iterator
    : public std::iterator<std::forward_iterator_tag, offset<IndexType, Rank>>
{
public:
    index_space_iterator() : index_space_(nullptr)
    {
    }

    explicit index_space_iterator(const index_space<IndexType, Rank>& index_space_)
    : index_space_(&index_space_)
    {
        for (long int i = 0; i < index_space_.rank(); ++i)
        {
            current_offset_[i] = index_space_.lower_bound(i);
        }
    }

    index_space_iterator<IndexType, Rank>& operator++()
    {
        QBB_ASSERT(index_space_, "Invalid iterator.");

        if (index_space_->rank() > 0)
        {
            for (long int dim = index_space_->rank(); dim-- > 0;)
            {
                current_offset_[dim] += index_space_->stride(dim);

                if (current_offset_[dim] < index_space_->upper_bound(dim))
                {
                    return *this;
                }
                else
                {
                    if (dim > 0)
                    {
                        current_offset_[dim] = index_space_->lower_bound(dim);
                    }
                    else
                    {
                        make_invalid();

                        return *this;
                    }
                }
            }
        }
        else
        {
            make_invalid();
        }

        return *this;
    }

    index_space_iterator<IndexType, Rank> operator++(int)
    {
        index_space_iterator<IndexType, Rank> old(*this);

        ++(*this);

        return old;
    }

    const offset<IndexType, Rank>& operator*() const
    {
        return current_offset_;
    }

    const offset<IndexType, Rank>* operator->() const
    {
        return &current_offset_;
    }

    friend bool operator==(const index_space_iterator<IndexType, Rank>& lhs,
                           const index_space_iterator<IndexType, Rank>& rhs)
    {
        return lhs.index_space_ == rhs.index_space_ && lhs.current_offset_ == rhs.current_offset_;
    }

private:
    void make_invalid()
    {
        current_offset_ = offset<IndexType, Rank>();
        index_space_ = nullptr;
    }

    offset<IndexType, Rank> current_offset_;
    const index_space<IndexType, Rank>* index_space_;
};

template <typename IndexType, long int Rank>
bool operator!=(const index_space_iterator<IndexType, Rank>& lhs,
                const index_space_iterator<IndexType, Rank>& rhs)
{
    return !(lhs == rhs);
}

template <typename IndexType, long int Rank>
index_space_iterator<IndexType, Rank> begin(const index_space<IndexType, Rank>& space)
{
    return index_space_iterator<IndexType, Rank>(space);
}

template <typename IndexType, long int Rank>
index_space_iterator<IndexType, Rank> end(const index_space<IndexType, Rank>& QBB_UNUSED(space))
{
    return index_space_iterator<IndexType, Rank>();
}

template <typename IndexType>
index_space<IndexType, 1> make_simple_index_space(IndexType lower_bound, IndexType upper_bound,
                                                  IndexType stride = 1)
{
    return index_space<IndexType, 1>({lower_bound, upper_bound, stride});
}

template <typename IndexType, std::size_t Rank>
index_space<IndexType, Rank> make_index_space_from_shape(const std::array<IndexType, Rank>& shape)
{
    std::array<std::array<IndexType, 3>, Rank> bounds;

    for (util::index_t i = 0; i < Rank; ++i)
    {
        bounds[i] = {{0, shape[i], 1}};
    }

    return index_space<IndexType, Rank>(std::move(bounds));
}

template <typename IndexType>
index_space<IndexType> make_index_space_from_shape(const std::vector<IndexType>& shape)
{
    std::vector<std::array<IndexType, 3>> bounds;

    for (const auto& extent : shape)
    {
        bounds.push_back({{0, extent, 1}});
    }

    return index_space<IndexType>(std::move(bounds));
}

template<typename IndexType, long int Rank>
IndexType linearize(const offset<IndexType, Rank>& off, const index_space<IndexType, Rank>& space)
{
    // TODO: Is this correct for a sliced index space?

    IndexType linear_index = 0;

    for (long int dim = 0; dim < space.rank(); ++dim)
    {
        linear_index += space.extent(dim) * linear_index + off[dim];
    }

    return linear_index;
}

template <typename IndexSpace, typename F>
void for_each(const IndexSpace& space, F f)
{
    for (const auto& offset : space)
    {
        f(offset);
    }
}
}
}

#endif
