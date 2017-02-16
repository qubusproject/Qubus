/*#include <qbb/qubus/backends/cpu_object_factory.hpp>

#include <limits>
#include <algorithm>
#include <functional>
#include <cstring>

inline namespace qbb
{
namespace qubus
{
    
cpu_object_factory::cpu_object_factory(allocator& allocator_, const abi_info& abi_info_)
: allocator_(&allocator_), abi_info_(&abi_info_)
{
}
    
std::unique_ptr<memory_block> cpu_object_factory::create_array(type value_type, std::vector<util::index_t> shape)
{
    auto size_type_size = abi_info_->get_size_of(types::integer());
    
    auto layout = abi_info_->get_array_layout(value_type, shape);
    
    auto rank = util::to_uindex(shape.size());

    auto mem_block = allocator_->allocate(layout.size(), layout.alignment());
    
    char* ptr = static_cast<char*>(mem_block->ptr());
    
    char* shape_ptr = ptr + layout.shape_offset();
    
    std::memcpy(ptr, &rank, size_type_size);
    
    std::memcpy(shape_ptr, shape.data(), layout.shape_size());
    
    return mem_block;
}

}
}*/