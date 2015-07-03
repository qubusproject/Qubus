#include <qbb/kubus/metadata_builder.hpp>

#include <qbb/kubus/logging.hpp>

#include <typeinfo>

namespace qbb
{
namespace qubus
{
void* build_array_metadata(const local_array& array, const local_address_space& addr_space,
                           const abi_info& abi, execution_stack& stack,
                           std::vector<std::shared_ptr<memory_block>>& used_mem_blocks)
{
    auto layout = abi.get_array_layout(array.value_type(), array.shape());

    logger slg;
 
    auto mem_block = addr_space.get_mem_block(array.address());

    void* base_addr = mem_block->ptr();
    
    BOOST_LOG_NAMED_SCOPE("memory");
    
    BOOST_LOG_SEV(slg, info) << "resolved address " << array.address() << " -> " << base_addr;
    
    void* data_addr = static_cast<char*>(base_addr) + layout.data_offset();
    void* shape_addr = static_cast<char*>(base_addr) + layout.shape_offset();
    
    used_mem_blocks.push_back(std::move(mem_block));
    
    return stack.emplace<array_metadata>(data_addr, shape_addr);
}

void* build_tensor_metadata(const local_tensor& tensor, const local_address_space& addr_space,
                            const abi_info& abi, execution_stack& stack,
                            std::vector<std::shared_ptr<memory_block>>& used_mem_blocks)
{
    return build_array_metadata(tensor.data(), addr_space, abi, stack, used_mem_blocks);
}

void* build_object_metadata(const object& obj, const local_address_space& addr_space,
                            const abi_info& abi, execution_stack& stack,
                            std::vector<std::shared_ptr<memory_block>>& used_mem_blocks)
{    
    if (obj.tag() == 0)
    {
        return build_tensor_metadata(static_cast<const local_tensor&>(obj), addr_space, abi, stack, used_mem_blocks);
    }
    else if(obj.tag() == 1)
    {
        return build_array_metadata(static_cast<const local_array&>(obj), addr_space, abi, stack, used_mem_blocks);
    }
    else
    {
        throw 0;
    }
}

}
}