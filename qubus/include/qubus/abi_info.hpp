#ifndef QUBUS_ABI_INFO_HPP
#define QUBUS_ABI_INFO_HPP

#include <qubus/IR/type.hpp>

#include <qubus/util/integers.hpp>
#include <qubus/util/unused.hpp>

#include <vector>
#include <cstddef>
#include <utility>

namespace qubus
{

//TODO: Make the members public.
class array_layout
{
public:
    array_layout() = default;

    array_layout(std::size_t size_, std::size_t alignment_, std::size_t shape_offset_, std::size_t shape_size_,
                 std::size_t data_offset_, std::size_t data_size_)
    : size_(size_), alignment_(alignment_), shape_offset_(shape_offset_), shape_size_(shape_size_),
      data_offset_(data_offset_), data_size_(data_size_)
    {
    }

    std::size_t size() const
    {
        return size_;
    }
    
    std::size_t shape_offset() const
    {
        return shape_offset_;
    }
    
    std::size_t data_offset() const
    {
        return data_offset_;
    }

    std::size_t shape_size() const
    {
        return shape_size_;
    }
    
    std::size_t data_size() const
    {
        return data_size_;
    }

    std::size_t alignment() const
    {
        return alignment_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & size_;
        ar & alignment_;
        ar & shape_offset_;
        ar & shape_size_;
        ar & data_offset_;
        ar & data_size_;
    }
private:
    std::size_t size_;
    std::size_t alignment_;
    std::size_t shape_offset_;
    std::size_t shape_size_;
    std::size_t data_offset_;
    std::size_t data_size_;
};

class abi_info
{
public:
    abi_info();

    std::size_t get_align_of(const type& primitive_type) const;
    std::size_t get_size_of(const type& primitive_type) const;

    array_layout get_array_layout(const type& value_type,
                                  const std::vector<util::index_t>& shape) const;

    template <typename Archive>
    void serialize(Archive& QBB_UNUSED(ar), unsigned QBB_UNUSED(version))
    {
    }
};
}

#endif