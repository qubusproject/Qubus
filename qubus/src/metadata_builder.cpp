#include <qbb/qubus/metadata_builder.hpp>

#include <qbb/qubus/logging.hpp>

namespace qbb
{
namespace qubus
{
void* build_array_metadata(const local_array& array, const abi_info& abi, execution_stack& stack,
                           std::vector<std::shared_ptr<memory_block>>& used_mem_blocks)
{
    auto layout = abi.get_array_layout(array.value_type(), array.shape());

    logger slg;

    auto mem_block = array.data_shared();

    void* base_addr = mem_block->ptr();

    // BOOST_LOG_NAMED_SCOPE("memory");

    // BOOST_LOG_SEV(slg, info) << "resolved address " << array.address() << " -> " << base_addr;

    void* data_addr = static_cast<char*>(base_addr) + layout.data_offset();
    void* shape_addr = static_cast<char*>(base_addr) + layout.shape_offset();

    used_mem_blocks.push_back(std::move(mem_block));

    return stack.emplace<array_metadata>(data_addr, shape_addr);
}

void* build_tensor_metadata(const local_tensor& tensor, const abi_info& abi, execution_stack& stack,
                            std::vector<std::shared_ptr<memory_block>>& used_mem_blocks)
{
    return build_array_metadata(tensor.data(), abi, stack, used_mem_blocks);
}

void* build_struct_metadata(const struct_& s, const abi_info& abi, execution_stack& stack,
                            std::vector<std::shared_ptr<memory_block>>& used_mem_blocks)
{
    void** addr = stack.emplace_array<void*>(s.members().size());

    void** member_addr = addr;
    for (const auto& member : s.members())
    {
        *member_addr = build_object_metadata(*member, abi, stack, used_mem_blocks);
        ++member_addr;
    }

    return addr;
}

void* build_object_metadata(const object& obj, const abi_info& abi, execution_stack& stack,
                            std::vector<std::shared_ptr<memory_block>>& used_mem_blocks)
{
    if (obj.tag() == 0)
    {
        return build_tensor_metadata(static_cast<const local_tensor&>(obj), abi, stack,
                                     used_mem_blocks);
    }
    else if (obj.tag() == 1)
    {
        return build_array_metadata(static_cast<const local_array&>(obj), abi, stack,
                                    used_mem_blocks);
    }
    else if (obj.tag() == 2)
    {
        return build_struct_metadata(static_cast<const struct_&>(obj), abi, stack, used_mem_blocks);
    }
    else
    {
        throw 0;
    }
}
}
}