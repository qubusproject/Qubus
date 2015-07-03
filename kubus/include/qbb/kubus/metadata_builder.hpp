#ifndef QBB_QUBUS_METADATA_BUILDER_HPP
#define QBB_QUBUS_METADATA_BUILDER_HPP

#include <qbb/kubus/local_array.hpp>
#include <qbb/kubus/local_tensor.hpp>
#include <qbb/kubus/object.hpp>

#include <qbb/kubus/backends/execution_stack.hpp>
#include <qbb/kubus/abi_info.hpp>
#include <qbb/kubus/local_address_space.hpp>

#include <vector>
#include <memory>

namespace qbb
{
namespace qubus
{

struct array_metadata
{
    void* data;
    void* shape;
};

void* build_array_metadata(const local_array& array, const local_address_space& addr_space,
                           const abi_info& abi, execution_stack& stack,
                           std::vector<std::shared_ptr<memory_block>>& used_mem_blocks);

void* build_tensor_metadata(const local_tensor& tensor, const local_address_space& addr_space,
                            const abi_info& abi, execution_stack& stack,
                            std::vector<std::shared_ptr<memory_block>>& used_mem_blocks);

void* build_object_metadata(const object& obj, const local_address_space& addr_space,
                            const abi_info& abi, execution_stack& stack,
                            std::vector<std::shared_ptr<memory_block>>& used_mem_blocks);
}
}

#endif