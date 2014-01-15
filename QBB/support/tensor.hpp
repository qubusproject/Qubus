#ifndef QBB_TENSOR_HPP
#define QBB_TENSOR_HPP

#include <QBB/support/integers.hpp>

#include <boost/range/numeric.hpp>

namespace qbb
{

template <typename T, int Rank>
class tensor
{
public:
    using value_type = T;
    
    tensor() = default;
    
    template <typename... Integers>
    explicit tensor(Integers... shape_)
    : shape_({to_uindex(shape_)...}),
      data_(boost::accumulate(this->shape_, 1, std::multiplies<index_t>()))
    {
        static_assert(Rank == sizeof...(shape_), "tensor: insufficient number of dimensions");
    }

    template <typename... Indices>
    T& operator()(Indices... indices)
    {
        static_assert(Rank == sizeof...(indices), "tensor: too many indices");

        index_t linear_index = 0;

        std::vector<index_t> indices_{to_uindex(indices)...};

        for (index_t i = 0; i < sizeof...(Indices); ++i)
        {
            linear_index = shape_[i] * linear_index + indices_[i];
        }

        assert(linear_index < to_uindex(data_.size()));
        assert(linear_index >= 0);

        return data_[linear_index];
    }

    template <typename... Indices>
    const T& operator()(Indices... indices) const
    {
        static_assert(Rank == sizeof...(indices), "tensor: insufficient number of indices");

        index_t linear_index = 0;

        std::vector<index_t> indices_{to_uindex(indices)...};

        for (index_t i = 0; i < sizeof...(Indices); ++i)
        {
            linear_index = shape_[i] * linear_index + indices_[i];
        }

        assert(linear_index < to_uindex(data_.size()));
        assert(linear_index >= 0);

        return data_[linear_index];
    }

    const std::vector<index_t>& shape() const
    {
        return shape_;
    }
    
    const T* data() const
    {
        return data_.data();
    }
    
    T* data()
    {
        return data_.data();
    }

    tensor<T, Rank>& operator+=(const tensor<T, Rank>& other)
    {
        assert(other.data_.size() == data_.size());

        for (index_t i = 0; i < to_uindex(data_.size()); ++i)
        {
            data_[i] += other.data_[i];
        }

        return *this;
    }

    tensor<T, Rank>& operator*=(T s)
    {
        for (index_t i = 0; i < to_uindex(data_.size()); ++i)
        {
            data_[i] *= s;
        }

        return *this;
    }

private:
    std::vector<index_t> shape_;
    std::vector<T> data_;
};

template <typename T, int Rank>
inline tensor<T, Rank> operator+(const tensor<T, Rank>& lhs, const tensor<T, Rank>& rhs)
{
    tensor<T, Rank> result(lhs);

    result += rhs;

    return result;
}

template <typename T, int Rank>
inline tensor<T, Rank> operator*(const tensor<T, Rank>& lhs, T rhs)
{
    tensor<T, Rank> result(lhs);

    result *= rhs;

    return result;
}

template <typename T, int Rank>
inline tensor<T, Rank> operator*(T lhs, const tensor<T, Rank>& rhs)
{
    tensor<T, Rank> result(rhs);

    result *= lhs;

    return result;
}

}

#endif