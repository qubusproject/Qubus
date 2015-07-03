#ifndef QBB_QUBUS_LOCAL_OBJECT_FACTORY_HPP
#define QBB_QUBUS_LOCAL_OBJECT_FACTORY_HPP

#include <qbb/qubus/memory_block.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/integers.hpp>

#include <vector>
#include <memory>

namespace qbb
{
namespace qubus
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