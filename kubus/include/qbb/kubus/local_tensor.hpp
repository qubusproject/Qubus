#ifndef QBB_QUBUS_LOCAL_TENSOR_HPP
#define QBB_QUBUS_LOCAL_TENSOR_HPP

#include <qbb/kubus/local_array.hpp>

#include <qbb/kubus/object.hpp>

#include <memory>
#include <utility>

namespace qbb
{
namespace qubus
{

class local_tensor final : public basic_object
{
public:
    explicit local_tensor(std::unique_ptr<local_array> data_) : data_(std::move(data_))
    {
    }
    
    virtual ~local_tensor()
    {
        destruct();
    }

    const local_array& data() const
    {
        return *data_;
    }
    
    type object_type() const override
    {
        return types::tensor(data_->value_type());
    }
    
    util::handle id() const override
    {
        return util::handle_from_ptr(this);
    }
    
    unsigned long int tag() const override
    {
        return 0;
    }
private:
    std::unique_ptr<local_array> data_;
};
}
}

#endif