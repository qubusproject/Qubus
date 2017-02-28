#ifndef QUBUS_OBJECT_HPP
#define QUBUS_OBJECT_HPP

#include <qubus/object_instance.hpp>
#include <qubus/local_address_space.hpp>

#include <qubus/distributed_future.hpp>
#include <qubus/object_monitor.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/naming.hpp>

#include <qubus/IR/type.hpp>

#include <qubus/util/handle.hpp>

namespace qubus
{

class object;

class object_server : public hpx::components::component_base<object_server>
{
public:
    object_server() = default;
    explicit object_server(type object_type_, std::size_t size_, std::size_t alignment_);
    ~object_server();

    object_server(const object_server&) = delete;
    object_server& operator=(const object_server&) = delete;
    
    type object_type() const;
    std::size_t size() const;
    std::size_t alignment() const;
    hpx::id_type id() const;

    object_instance primary_instance() const;

    bool has_data() const;

    token acquire_read_access();
    token acquire_write_access();

    std::vector<object> components() const;

    HPX_DEFINE_COMPONENT_ACTION(object_server, object_type, object_type_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, size, size_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, alignment, alignment_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, primary_instance, primary_instance_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, has_data, has_data_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, acquire_read_access, acquire_read_access_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, acquire_write_access, acquire_write_access_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, components, components_action);

    void register_instance(local_address_space::handle data);
    void add_component(const object& component);
    const object& operator()(long int index) const;
private:
    object_monitor monitor_;

    type object_type_;
    std::size_t size_;
    std::size_t alignment_;
    local_address_space::handle data_;
    std::vector<object> components_;
};

class object : public hpx::components::client_base<object, object_server>
{
public:
    using base_type = hpx::components::client_base<object, object_server>;

    object() = default;
    object(hpx::id_type&& id);
    object(hpx::future<hpx::id_type>&& id);

    type object_type() const;
    std::size_t size() const;
    std::size_t alignment() const;

    hpx::id_type id() const;

    object_instance primary_instance() const;

    bool has_data() const;

    token acquire_read_access();
    token acquire_write_access();

    std::vector<object> components() const;

    friend bool operator==(const object& lhs, const object& rhs)
    {
        return lhs.id() == rhs.id();
    }

    friend bool operator!=(const object& lhs, const object& rhs)
    {
        return !(lhs == rhs);
    }
};

}

#endif