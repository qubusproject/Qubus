#ifndef QUBUS_UTIL_SPAN_HPP
#define QUBUS_UTIL_SPAN_HPP

#include <cstddef>

namespace qubus
{
namespace util
{

template <typename T>
class span
{
public:
    using pointer = T*;
    using reference = T&;
    using iterator = pointer;
    using const_iterator = const pointer;

    constexpr span(pointer data_, std::size_t size_)
    : data_(data_), size_(size_)
    {
    }

    constexpr reference operator[](std::size_t index)
    {
        return data()[index];
    }

    constexpr pointer data() const noexcept
    {
        return data_;
    }

    constexpr std::size_t size() const noexcept
    {
        return size_;
    }

    constexpr iterator begin() const noexcept
    {
        return data_;
    }

    constexpr iterator end() const noexcept
    {
        return data_ + size_;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return data_;
    }

    constexpr const_iterator cend() const noexcept
    {
        return data_ + size_;
    }
private:
    pointer data_;
    std::size_t size_;
};

}
}

#endif
