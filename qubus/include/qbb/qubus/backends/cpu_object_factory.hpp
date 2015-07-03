#ifndef QBB_QUBUS_CPU_OBJECT_FACTORY_HPP
#define QBB_QUBUS_CPU_OBJECT_FACTORY_HPP

#include <qbb/qubus/local_object_factory.hpp>

#include <qbb/qubus/backends/cpu_allocator.hpp>
#include <qbb/qubus/memory_block.hpp>
#include <qbb/qubus/IR/type.hpp>
#include <qbb/qubus/abi_info.hpp>

#include <qbb/util/integers.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class cpu_object_factory final : public local_object_factory
{
public:
    cpu_object_factory(allocator& allocator_, const abi_info& abi_info_);
    
    std::unique_ptr<memory_block> create_array(type value_type, std::vector<util::index_t> shape) override;

private:
    allocator* allocator_;
    const abi_info* abi_info_;
};
}
}

#endif