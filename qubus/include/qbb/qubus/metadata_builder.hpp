#ifndef QBB_QUBUS_METADATA_BUILDER_HPP
#define QBB_QUBUS_METADATA_BUILDER_HPP

#include <qbb/qubus/object.hpp>

#include <qbb/qubus/jit/execution_stack.hpp>
#include <qbb/qubus/abi_info.hpp>
#include <qbb/qubus/local_address_space.hpp>

#include <vector>
#include <array>
#include <memory>

namespace qbb
{
namespace qubus
{

/*void* build_array_metadata(const local_array& array, const abi_info& abi, execution_stack& stack,
                           std::vector<std::shared_ptr<memory_block>>& used_mem_blocks);

void* build_tensor_metadata(const local_tensor& tensor, const abi_info& abi, execution_stack& stack,
                            std::vector<std::shared_ptr<memory_block>>& used_mem_blocks);

void* build_struct_metadata(const struct_& s, const abi_info& abi, execution_stack& stack,
                            std::vector<std::shared_ptr<memory_block>>& used_mem_blocks);

void* build_object_metadata(const object& obj, const abi_info& abi, execution_stack& stack,
                            std::vector<std::shared_ptr<memory_block>>& used_mem_blocks);*/
}
}

#endif