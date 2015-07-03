#ifndef QBB_QUBUS_LOCAL_ARRAY_HPP
#define QBB_QUBUS_LOCAL_ARRAY_HPP

#include <qbb/qubus/object.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/util/integers.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class local_array final : public basic_object
{
public:
    explicit local_array(util::handle address_, type value_type_, std::vector<util::index_t> shape_)
    : address_(address_), value_type_(value_type_), shape_(shape_)
    {
    }
    
    virtual ~local_array()
    {
        destruct();
    }
    
    const util::handle& address() const
    {
        return address_;
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
    util::handle address_;
    type value_type_;
    std::vector<util::index_t> shape_;
};

}
}

#endif