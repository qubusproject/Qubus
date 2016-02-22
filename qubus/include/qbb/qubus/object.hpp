#ifndef QBB_QUBUS_OBJECT_HPP
#define QBB_QUBUS_OBJECT_HPP

#include <hpx/include/lcos.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/handle.hpp>

namespace qbb
{
namespace qubus
{

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

    virtual hpx::shared_future<void> get_last_modification() const = 0;
    virtual void record_modification(hpx::shared_future<void> f) = 0;
};

class basic_object : public object
{
public:
    basic_object();

    virtual ~basic_object() = default;

    hpx::shared_future<void> get_last_modification() const;

    void record_modification(hpx::shared_future<void> modification);
private:
    hpx::shared_future<void> last_modification_;
};
    
}
}
#endif