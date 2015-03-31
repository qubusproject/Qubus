#ifndef QBB_KUBUS_LOCAL_OBJECT_FACTORY_HPP
#define QBB_KUBUS_LOCAL_OBJECT_FACTORY_HPP

#include <qbb/kubus/memory_block.hpp>

#include <qbb/kubus/IR/type.hpp>

#include <qbb/util/integers.hpp>

#include <vector>
#include <memory>

namespace qbb
{
namespace kubus
{

class local_object_factory
{
public:
    local_object_factory() = default;
    virtual ~local_object_factory() = default;
    
    local_object_factory(const local_object_factory&) = delete;
    local_object_factory& operator=(const local_object_factory&) = delete;
    
    virtual std::unique_ptr<memory_block> create_array(type value_type, std::vector<util::index_t> shape) = 0;
};
    
}
}

#endif