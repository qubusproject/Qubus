#ifndef QUBUS_VIRTUAL_ADDRESS_SPACE_HPP
#define QUBUS_VIRTUAL_ADDRESS_SPACE_HPP

#include <qubus/object.hpp>
#include <qubus/object_instance.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <memory>

namespace qubus
{

class virtual_address_space
{
public:
    virtual ~virtual_address_space() = default;

    virtual hpx::future<object_instance> resolve_object(const object& obj) = 0;
    virtual object_instance try_resolve_object(const object& obj) const = 0;
};

class virtual_address_space_wrapper : public hpx::components::component_base<virtual_address_space_wrapper>
{
public:
    class client : public virtual_address_space, public hpx::components::client_base<client, virtual_address_space_wrapper>
    {
    public:
        using base_type = hpx::components::client_base<client, virtual_address_space_wrapper>;

        client() = default;

        client(hpx::id_type&& id);
        client(hpx::future<hpx::id_type>&& id);

        ~client() override = default;

        hpx::future<object_instance> resolve_object(const object& obj) override;
        object_instance try_resolve_object(const object& obj) const override;
    };

    virtual_address_space_wrapper() = default;
    explicit virtual_address_space_wrapper(std::unique_ptr<virtual_address_space> wrapped_address_space_);

    hpx::future<object_instance> resolve_object(const object& obj);
    object_instance try_resolve_object(const object& obj) const;

    HPX_DEFINE_COMPONENT_ACTION(virtual_address_space_wrapper, resolve_object, resolve_object_action);
    HPX_DEFINE_COMPONENT_ACTION(virtual_address_space_wrapper, try_resolve_object, try_resolve_object_action);

private:
    std::unique_ptr<virtual_address_space> wrapped_address_space_;
};

}

#endif
