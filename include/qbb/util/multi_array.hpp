#ifndef MULTI_ARRAY_HPP
#define MULTI_ARRAY_HPP

#include <vector>
#include <array>
#include <algorithm>
#include <functional>

template <typename T, int Rank>
class multi_array
{
public:
    typedef T value_type;

    multi_array() = default;

    explicit multi_array(const std::vector<std::size_t>& shape_, const T& value_ = T())
    : data_(std::accumulate(std::begin(shape_), std::end(shape_), std::size_t(1),
                            std::multiplies<std::size_t>()),
            value_),
      shape_(shape_)
    {
    }

    template <typename... Indices>
    const T& operator()(Indices... indices) const
    {
        return operator()({static_cast<long int>(indices)...});
    }

    template <typename... Indices>
    T& operator()(Indices... indices)
    {        
        return operator()({static_cast<long int>(indices)...});
    }

    const T& operator()(const std::array<std::size_t, Rank>& indices) const
    {
        long int linear_index = 0;
        
        for (long int i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices[i];
        }
        
        return data_[linear_index];
    }

    template <typename... Indices>
    T& operator()(const std::array<std::size_t, Rank>& indices)
    {
        long int linear_index = 0;
        
        for (long int i = 0; i < Rank; ++i)
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

    const std::array<std::size_t, Rank>& shape() const
    {
        return shape_;
    }

    void reshape(const std::array<std::size_t, Rank>& new_shape)
    {
        data_.resize(std::accumulate(std::begin(new_shape), std::end(new_shape), std::size_t(1),
                                     std::multiplies<std::size_t>()));

        shape_ = new_shape;
    }

private:
    std::vector<T> data_;
    std::array<std::size_t, Rank> shape_;
};


#endif