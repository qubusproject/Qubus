#ifndef QBB_KUBUS_OBJECT_HPP
#define QBB_KUBUS_OBJECT_HPP

#include <qbb/kubus/IR/type.hpp>

#include <boost/signals2.hpp>

#include <qbb/util/handle.hpp>

namespace qbb
{
namespace qubus
{

class object;
    
using object_predestructor_signal = boost::signals2::signal<void(const object&)>;

class object
{
public:
    object() = default;
    virtual ~object() = default;
    
    object(const object&) = delete;
    object& operator=(const object&) = delete;
    
    virtual type object_type() const = 0;
    virtual util::handle id() const = 0;
    virtual unsigned long int tag() const = 0;
    
    virtual void on_destruction(const object_predestructor_signal::slot_type& subscriber) const = 0;
};

class basic_object : public object
{
public:
    virtual ~basic_object() = default;
    
    void on_destruction(const object_predestructor_signal::slot_type& subscriber) const override final;
private:
    mutable object_predestructor_signal on_destruction_;
protected:
    void destruct() const;
};
    
}
}
#endif