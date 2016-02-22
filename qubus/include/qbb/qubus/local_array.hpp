#ifndef QBB_QUBUS_LOCAL_ARRAY_HPP
#define QBB_QUBUS_LOCAL_ARRAY_HPP

#include <qbb/qubus/object.hpp>

#include <qbb/qubus/memory_block.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/integers.hpp>

#include <vector>
#include <utility>

namespace qbb
{
namespace qubus
{

class local_array final : public basic_object
{
public:
    explicit local_array(std::unique_ptr<memory_block> data_, type value_type_, std::vector<util::index_t> shape_)
    : data_(std::move(data_)), value_type_(value_type_), shape_(shape_)
    {
    }
    
    virtual ~local_array() = default;
    
    const memory_block& data() const
    {
        return *data_;
    }

    std::shared_ptr<memory_block> data_shared() const
    {
        return data_;
    }
    
    const type& value_type() const
    {
        return value_type_;
    }
    
    const std::vector<util::index_t>& shape() const
    {
        return shape_;
    }
    
    util::index_t rank() const
    {
        return util::to_uindex(shape_.size());
    }
    
    type object_type() const override
    {
        return types::array(value_type());
    }
    
    util::handle id() const override
    {
        return util::handle_from_ptr(this);
    }
    
    unsigned long int tag() const override
    {
        return 1;
    }
private:
    std::shared_ptr<memory_block> data_;
    type value_type_;
    std::vector<util::index_t> shape_;
};

}
}

#endif