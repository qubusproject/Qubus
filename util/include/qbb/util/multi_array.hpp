#ifndef QBB_UTIL_MULTI_ARRAY_HPP
#define QBB_UTIL_MULTI_ARRAY_HPP

#include <qbb/util/integers.hpp>
#include <qbb/util/detail/all_integral.hpp>
#include <qbb/util/index_space.hpp>

#include <vector>
#include <array>
#include <type_traits>
#include <algorithm>
#include <functional>

namespace qubus
{
namespace util
{

template <typename T, index_t Rank>
class multi_array
{
public:
    typedef T value_type;

    multi_array() = default;

    explicit multi_array(const std::vector<index_t>& shape_, const T& value_ = T())
    : data_(std::accumulate(std::begin(shape_), std::end(shape_), index_t(1),
                            std::multiplies<index_t>()),
            value_),
      shape_(shape_)
    {
    }

    explicit multi_array(const std::array<index_t, Rank>& shape_, const T& value_ = T())
            : data_(std::accumulate(std::begin(shape_), std::end(shape_), index_t(1),
                                    std::multiplies<index_t>()),
                    value_),
              shape_(shape_)
    {
    }

    template <typename... Indices, typename Enabler = typename std::enable_if<
                                       detail::all_integral<Indices...>::value>::type>
    const T& operator()(Indices... indices) const
    {
        static_assert(sizeof...(Indices) == Rank, "Invalid number of indices.");

        return operator()({{to_uindex(indices)...}});
    }

    template <typename... Indices, typename Enabler = typename std::enable_if<
                                       detail::all_integral<Indices...>::value>::type>
    T& operator()(Indices... indices)
    {
        static_assert(sizeof...(Indices) == Rank, "Invalid number of indices.");

        return operator()({{to_uindex(indices)...}});
    }

    const T& operator()(const std::array<index_t, Rank>& indices) const
    {
        index_t linear_index = 0;

        for (index_t i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices[i];
        }

        return data_[linear_index];
    }

    T& operator()(const std::array<index_t, Rank>& indices)
    {
        index_t linear_index = 0;

        for (index_t i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices[i];
        }

        return data_[linear_index];
    }

    const T& operator()(const offset<index_t, Rank>& indices) const
    {
        index_t linear_index = 0;

        for (index_t i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices[i];
        }

        return data_[linear_index];
    }

    T& operator()(const offset<index_t, Rank>& indices)
    {
        index_t linear_index = 0;

        for (index_t i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices[i];
        }

        return data_[linear_index];
    }

    const T* data() const
    {
        return data_.data();
    }

    T* data()
    {
        return data_.data();
    }

    typename std::vector<T>::iterator begin()
    {
        return data_.begin();
    }

    typename std::vector<T>::iterator end()
    {
        return data_.end();
    }

    typename std::vector<T>::const_iterator begin() const
    {
        return data_.begin();
    }

    typename std::vector<T>::const_iterator end() const
    {
        return data_.end();
    }

    const std::array<index_t, Rank>& shape() const
    {
        return shape_;
    }

    void reshape(const std::array<index_t, Rank>& new_shape)
    {
        data_.resize(std::accumulate(std::begin(new_shape), std::end(new_shape), index_t(1),
                                     std::multiplies<index_t>()));

        shape_ = new_shape;
    }

private:
    std::vector<T> data_;
    std::array<index_t, Rank> shape_;
};
}
}

#endif