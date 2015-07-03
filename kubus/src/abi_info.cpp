#include <qbb/kubus/abi_info.hpp>

#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/type.hpp>

#include <qbb/util/unused.hpp>
#include <qbb/util/integers.hpp>

namespace qbb
{
namespace qubus
{

abi_info::abi_info()
{
}

std::size_t abi_info::get_align_of(const type& QBB_UNUSED(primitive_type)) const
{
    return 32;
}

std::size_t abi_info::get_size_of(const type& primitive_type) const
{
    pattern::variable<type> real_type;

    auto m = pattern::make_matcher<type, std::size_t>()
                 .case_(pattern::double_t,
                        [&]
                        {
                            return sizeof(double);
                        })
                 .case_(pattern::float_t,
                        [&]
                        {
                            return sizeof(float);
                        })
                 .case_(pattern::integer_t,
                        [&]
                        {
                            return sizeof(util::index_t);
                        })
                 .case_(pattern::bool_t,
                   [&]
                   {
                       return sizeof(bool);
                   })
                 .case_(complex_t(real_type), [&]
                        {
                            return 2 * get_size_of(real_type.get());
                        });

    return pattern::match(primitive_type, m);
}

array_layout abi_info::get_array_layout(const type& value_type,
                                        const std::vector<util::index_t>& shape) const
{
    auto value_type_size = get_size_of(value_type);
    auto size_type_size = get_size_of(types::integer());
    auto data_alignment = get_align_of(value_type);
    auto shape_alignment = get_align_of(types::integer()); 
    
    auto rank = shape.size();
    
    // The alignment of the entire block needs to be a multiple of the
    // data alignment and the shape alignment.
    auto block_alignment = std::max(data_alignment, shape_alignment);
    
    auto shape_size = rank * size_type_size;
    
    auto data_size = std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<std::size_t>()) * value_type_size;
    
    util::index_t meta_data_size = size_type_size;
 
    std::size_t current_size = meta_data_size;
    
    auto shape_padding = shape_alignment - current_size % shape_alignment;
    
    current_size += shape_padding + shape_size;
    
    auto data_padding = data_alignment - current_size % data_alignment;
    
    current_size += data_padding + data_size;
    
    auto shape_offset = meta_data_size + shape_padding;
    auto data_offset = shape_offset + shape_size + data_padding;
    
    return array_layout(current_size, block_alignment, shape_offset, shape_size, data_offset, data_size);
}
}
}