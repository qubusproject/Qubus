#ifndef QBB_QUBUS_OBJECT_HPP
#define QBB_QUBUS_OBJECT_HPP

#include <hpx/include/lcos.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/naming.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/handle.hpp>

namespace qbb
{
namespace qubus
{

class object : public hpx::components::abstract_component_base<object>
{
public:
    object() = default;
    virtual ~object() = default;
    
    object(const object&) = delete;
    object& operator=(const object&) = delete;
    
    virtual type object_type() const = 0;
    virtual hpx::id_type id() const = 0;

    virtual hpx::shared_future<void> get_last_modification() const = 0;
    virtual void record_modification(hpx::shared_future<void> f) = 0;

    type object_type_nonvirt() const { return object_type(); }
    HPX_DEFINE_COMPONENT_ACTION(object, object_type_nonvirt, object_type_action);

    hpx::shared_future<void> get_last_modification_nonvirt() const { return get_last_modification(); }
    HPX_DEFINE_COMPONENT_ACTION(object, get_last_modification_nonvirt, get_last_modification_action);

    void record_modification_nonvirt(hpx::shared_future<void> f) { record_modification(f); }
    HPX_DEFINE_COMPONENT_ACTION(object, record_modification_nonvirt, record_modification_action);
};

class basic_object : public object, hpx::components::abstract_component_base<basic_object>
{
public:
    basic_object();

    virtual ~basic_object() = default;

    hpx::shared_future<void> get_last_modification() const;

    void record_modification(hpx::shared_future<void> modification);
private:
    hpx::shared_future<void> last_modification_;
};

class object_client : public hpx::components::client_base<object_client, object>
{
public:
    using base_type = hpx::components::client_base<object_client, object>;

    object_client() = default;
    object_client(hpx::future<hpx::id_type>&& id);

    type object_type() const;

    hpx::id_type id() const;

    hpx::shared_future<void> get_last_modification() const;

    void record_modification(hpx::shared_future<void> f);

    friend bool operator==(const object_client& lhs, const object_client& rhs)
    {
        return lhs.id() == rhs.id();
    }

    friend bool operator!=(const object_client& lhs, const object_client& rhs)
    {
        return !(lhs == rhs);
    }
};

}
}

#endif